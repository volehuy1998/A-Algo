#include <bits/stdc++.h>
#include <SDL2/SDL.h>
using namespace std;

SDL_Event event;
SDL_Window  * window   = NULL;
SDL_Renderer* renderer = NULL;

bool isrunning = true;
bool isWall    = false;
bool found     = false;
bool stop      = true;
constexpr int CENTER 	 = SDL_WINDOWPOS_CENTERED;
constexpr int EXUBERANCY = 2;
constexpr int WIDTH  	 = 600 - EXUBERANCY;
constexpr int HEIGHT 	 = 600 - EXUBERANCY;
constexpr int edge 	 = 10;
constexpr int cols 	 = WIDTH  / edge;
constexpr int rows 	 = HEIGHT / edge;

void quit() {
	isrunning = false;
}

struct random_machine {
	static random_device device;
	static mt19937 gen;
	static uniform_int_distribution<> dis;
	static int get() {
		return dis(gen);
	}
};
random_device random_machine::device;
mt19937 random_machine::gen(random_machine::device());
uniform_int_distribution<int> random_machine::dis(0, 100);

struct Node {
	int i, j;
	int g, h, f;
	SDL_Rect rect;
	vector<Node*> childs;
	Node * parent;
	bool wall = false;
	Node () {
		if (random_machine::get() < 15) { // [min,max] = [0,100]
			wall = true;
		}
		i = j = h = 0;
		g = f = 99999;
		rect.x = rect.y =  EXUBERANCY;
		rect.w = rect.h = -EXUBERANCY;
		parent = nullptr;
	}
	Node (const Node& node) {
		this->i = node.i;
		this->j = node.j;
		this->g = node.g;
		this->h = node.h;
		this->f = node.f;
		this->rect.x = node.rect.x;
		this->rect.y = node.rect.y;
		this->rect.w = node.rect.w;
		this->rect.h = node.rect.h;
		this->parent = node.parent;
		for (auto& child : node.childs) {
			this->childs.emplace_back(child);
		}
	}
	bool operator== (const Node& node) {
		if (this != &node) {
			if (this->i == node.i && this->j == node.j) {
				return true;
			} else {
				return false;
			}
		}
		return true;
	}
	Node& operator= (const Node& node) {
		if (this != &node) {
			this->i = node.i;
			this->j = node.j;
			this->g = node.g;
			this->h = node.h;
			this->f = node.f;
			this->rect.x = node.rect.x;
			this->rect.y = node.rect.y;
			this->rect.w = node.rect.w;
			this->rect.h = node.rect.h;
			this->parent = node.parent;
			for (auto& child : node.childs) {
				this->childs.emplace_back(child);
			}
		}
		return *this;
	}
	void addChilds(Node grid[][cols]) {
		int i = this->i;
		int j = this->j;
		if (i - 1 >= 0) {
			this->childs.emplace_back(&grid[i-1][j]);
		}
		if (i + 1 < rows) {
			this->childs.emplace_back(&grid[i+1][j]);
		}
		if (j - 1 >= 0) {
			this->childs.emplace_back(&grid[i][j-1]);
		}
		if (j + 1 < cols) {
			this->childs.emplace_back(&grid[i][j+1]);
		}
		if (i - 1 >= 0 && j - 1 >= 0) {
			this->childs.emplace_back(&grid[i-1][j-1]);
		}
		if (i + 1 < rows && j + 1 < cols) {
			this->childs.emplace_back(&grid[i+1][j+1]);
		}
		if (i - 1 >= 0 && j + 1 < cols) {
			this->childs.emplace_back(&grid[i-1][j+1]);
		}
		if (i + 1 < rows && j - 1 >= 0) {
			this->childs.emplace_back(&grid[i+1][j-1]);
		}
	}
	void setij(int i, int j) {
		this->i = i;
		this->j = j;
		this->rect.x += j * edge;
		this->rect.y += i * edge;
		this->rect.w += edge;
		this->rect.h += edge;
	}
	void show(int r, int g, int b) {
		SDL_SetRenderDrawColor(renderer, r, g, b, 0xff);
		if (this->wall) {
			SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0xff);
		}
		SDL_RenderFillRect(renderer, &rect);
	}
	friend bool operator< (const Node& left, const Node& right);
};

bool operator< (const Node& left, const Node& right)
{
    return left.f < right.f;
}

auto h = [](Node& A, Node& B) {
	int x1 = A.rect.x;
	int y1 = A.rect.y;
	int x2 = B.rect.x;
	int y2 = B.rect.y;
	return sqrt(abs((x1 - x2) * (x1 - x2)) + abs((y1 - y2) * (y1 - y2)));
};

int main(int argc, char ** argv) {
	cout << left;
	cout << setw(27) << "Type 'q' to quit\t\t\t";
	cout << setw(27) << "Type 'a' to search\t\t\t";
	cout << setw(27) << "Type 's' to stop" << endl;
	cout << setw(27) << "Type 'w' to create walls\t\t\t";
	cout << setw(27) << "Type 'r' to remove\t\t\t";
	cout << setw(27) << "Type 'c' to change map" << endl;
	cout << setw(27) << "Type 'b' to back" << endl;

	SDL_Init(SDL_INIT_EVERYTHING);	
	window = SDL_CreateWindow("SDL2", 0, 0, WIDTH, HEIGHT, SDL_WINDOW_FULLSCREEN_DESKTOP);
	renderer = SDL_CreateRenderer(window, -1, 0);
	Node grid[rows][cols];
	#pragma omp for
	for (int i = 0; i < rows; i++) {
		//#pragma omp for
		for (int j = 0; j < cols; j++) {
			grid[i][j].setij(i, j);
		}
	}
	#pragma omp for
	for (int i = 0; i < rows; i++) {
		//#pragma omp for
		for (int j = 0; j < cols; j++) {
			grid[i][j].addChilds(grid);
		}
	}
	/* astart */
	Node &start_node = grid[1][1];
	Node &end_node = grid[rows - 1][cols - 1];
	start_node.wall = false;
	start_node.f = h(start_node, end_node); 
	end_node.wall = false;
	vector<reference_wrapper<Node>> open_list;
	vector<Node> closed_list;
	vector<Node> path;
	vector<Node*> walls;
	open_list.emplace_back(start_node);

	start_node.show(0xff, 0xff, 0x0);
	end_node.show(0xff, 0xff, 0x0);
	auto back = [&] {
		path.clear();
		open_list.clear();
		closed_list.clear();
		stop = true;
		found = false;
		open_list.emplace_back(start_node);
		#pragma omp for
	  for (auto& node_ptr : walls) {
	 	  node_ptr->wall = false;
	  }
		walls.clear();
	};
	while (isrunning) {
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym) {
						case SDLK_q: quit(); break;
						case SDLK_a: stop = false;     break;
						case SDLK_s: stop = true;      break;
						case SDLK_w: isWall = !isWall; break; 
						case SDLK_b: {
								back();
							     } break;
						case SDLK_r: {
							        back();
								for (auto& arr : grid) {
									for (auto& node : arr) {
										node.wall = false;
									 }
								}
							     } break;
						case SDLK_c: {
								back();
								static int i = 10;
								for (auto& arr : grid) {
									for (auto& node : arr) {
										node.wall = false;
									}
								}
								for (auto& arr : grid) {
									for (auto& node : arr) {
										if (random_machine::get() < (i % 60)) {
											node.wall = true;
										}
									}
								}
								i += 10;
								} break;
					} break;
				case SDL_MOUSEMOTION:
					if (isWall && found == false) {
						int x = event.motion.x;
						int y = event.motion.y;
						if ((x >= EXUBERANCY && x < WIDTH  - EXUBERANCY) && 
								(y >= EXUBERANCY && y < HEIGHT - EXUBERANCY)) {
							grid[y / edge][x / edge].wall = true;
							walls.emplace_back(&grid[y / edge][x / edge]);
						}
					} break;
				case SDL_MOUSEBUTTONDOWN:
					if (found == false) {
						int x = event.motion.x;
						int y = event.motion.y;
						grid[y / edge][x / edge].wall = !grid[y / edge][x / edge].wall;
					}
					break;
			}
		}
		start_node.wall = end_node.wall = false;
		Node current;
		if (stop == false) {
			if (found == false) {
				if (!open_list.empty()) {
					sort(open_list.begin(), open_list.end(), [] (Node& l, Node& r) { return l.f < r.f; });
					Node& current_node = open_list[0];
					current = current_node;
					open_list.erase(open_list.begin());
					closed_list.emplace_back(current_node);
					if (current_node == end_node) {
						found = true;
					}
					for (auto& child : current_node.childs) {
						if ([&] { 
							for (auto& closed_child : closed_list) {
								if (*child == closed_child) {
									return true;
								}
							}
							return false;
						}() || child->wall) {
							continue;
						}
						int g_temp = current_node.g + h(current_node, *child);
						if ([&] {
							for (auto& open_node : open_list) {
								if (*child == open_node) {
									return true;
								}
							}
							return false;
						}() == false) {
							open_list.emplace_back(*child);
						} else if (g_temp >= child->g) {
							continue;	
						}
						child->parent = &current_node;
						child->h = h(*child, end_node);
						child->g = g_temp;
						child->f = child->g + child->h;
					}
				}
				path.clear();
				for (Node node = current; node.parent != nullptr; node = *(node.parent)) {
					path.emplace_back(node);
				}
				path.emplace_back(start_node);
			}
		}
		#pragma omp parallel
		{
			#pragma omp for 
			for (auto& arr : grid) {
				for (auto& node : arr) {
					node.show(0xff, 0xff, 0xff);
				}
			}
			#pragma omp for 
			for (auto& node : closed_list) {
				node.show(0xff, 117, 80);
			}
			#pragma omp for 
			for (auto& node : open_list) {
				node.get().show(0x0, 0xff, 0x0);
			}
			#pragma omp for 
			for (auto& node : path) {
				node.show(0x0, 0x0, 0xff);
			}
		}
		end_node.show(0xff, 0xff, 0x0);
		start_node.show(0xff, 0xff, 0x0);
		SDL_RenderPresent(renderer);
	}
	/* end astart */
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
