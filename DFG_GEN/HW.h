#pragma once
#include "PU.h"
#include <vector>

class HardwareResource {
public:
	int Width, Height;
	int num_of_processor;
	std::vector<ProcessorUnit> resources;
	HardwareResource(int w, int h): Width(w), Height(h) {
		int id = 0;
		for (int i = 0; i < Height;i++) { // i for row
			for (int j = 0; j < Width;j++) { // j for column
				int N, W, S, E, L;
				N = W = S = E = L = 1;
				if (i == 0)
					N = 0;
				if (j == 0)
					W = 0;
				if (i == (Height - 1))
					S = 0;
				if (j == (Width - 1))
					E = 0;
				Channel cur_ch(N, W, S, E, L);
				resources.push_back(ProcessorUnit(j, i, cur_ch, id++));
			}
		}
		num_of_processor = resources.size();
	};
	int getMaxWD() { return (Width + Height) * 2; };
	HardwareResource(): Width(0), Height(0), resources(0), num_of_processor(0) {};
};