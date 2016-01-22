#pragma once
class Channel {
	Channel::Channel(int n, int w, int s, int e, int l) : N(n), W(w), S(s), E(e), L(l) {};
	Channel::Channel(int val) : N(val), W(val), S(val), E(val), L(val) {};
	Channel::Channel(): N(1), W(1), S(1), E(1), L(1) {};
	friend class ProcessorUnit;
	friend class HardwareResource;
	int N, W, S, E, L;
};

class ProcessorUnit {
public:
	int loc_x, loc_y; //x for column, y for row
	Channel CH;
	ProcessorUnit::ProcessorUnit(): loc_x(0), loc_y(0), CH(0) {};
	ProcessorUnit::ProcessorUnit(int x, int y, Channel ch): loc_x(x), loc_y(y), CH(ch) {};
};