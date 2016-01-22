#pragma once
#include "PU.h"
#include <vector>
#include "Scheduler.h"

class HardwareResource {
public:
	int Width, Height;
	std::vector<ProcessorUnit> resources;
	HardwareResource::HardwareResource(int w, int h): Width(w), Height(h) {
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
				resources.push_back(ProcessorUnit(j, i, cur_ch));
			}
		}
	};






	HardwareResource::HardwareResource(): Width(0), Height(0), resources(0) {};
};