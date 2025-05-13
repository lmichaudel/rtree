#pragma once

#include "dataset.h"
#include "gfx.h"
#include "rtree.h"

int main(void);
void construct_rtree(void);
void clear_rtree(void);
void start_gui(void);
void draw_node(Node* node, int d);
void draw(void);
