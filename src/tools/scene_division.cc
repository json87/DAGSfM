// Copyright (c) 2018, ETH Zurich and UNC Chapel Hill.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//
//     * Neither the name of ETH Zurich and UNC Chapel Hill nor the names of
//       its contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Author: Johannes L. Schoenberger (jsch-at-demuc-dot-de)

#include "base/database.h"
#include "util/logging.h"
#include "util/math.h"
#include "util/misc.h"
#include "util/option_manager.h"
#include "clustering/ncut_cluster.h"

#include <fstream>

using namespace colmap;
using namespace DAGSfM;

// Simple example that reads and writes a reconstruction.
int main(int argc, char** argv) {
	InitializeGlog(argv);

	std::string input_path;
	int num_partitions = 5;
	std::string output_path;

	OptionManager options;
	options.AddDatabaseOptions();
	options.AddRequiredOption("input_path", &input_path);
	options.AddDefaultOption("num_partitions", &num_partitions);
	options.AddRequiredOption("output_path", &output_path);
	options.Parse(argc, argv);

	// Read image pairs and calculate weights.
	std::vector<std::pair<int, int>> view_pairs;
	std::vector<int> weights;

	std::ifstream file_input(input_path);
	CHECK(file_input.is_open()) << input_path;

	std::string line, item;

	while (std::getline(file_input, line)) {
		StringTrim(&line);

		if (line.empty() || line[0] == '#') {
			continue;
		}

		std::stringstream line_stream(line);

		std::getline(line_stream, item, ' ');
		int id1 = std::stoi(item);
		std::getline(line_stream, item, ' ');
		int id2 = std::stoi(item);
		view_pairs.push_back(std::pair<int, int>(id1, id2));

		std::getline(line_stream, item, ' ');
		weights.push_back(100*std::stod(item));
	}

	CHECK_EQ(view_pairs.size(), weights.size());

	// Scene cluster based on NormalizedCut.
	NCutCluster cluster;
	cluster.InitIGraph(view_pairs, weights);
	cluster.ComputeCluster(view_pairs, weights, num_partitions);

	// Prepare clustered scenes.
	CreateDirIfNotExists(output_path);
	cluster.OutputIGraph(output_path);

	return EXIT_SUCCESS;
}
