#ifndef VIEWER_HPP
#define VIEWER_HPP
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <vector>
#include <cmath>
#include <algorithm>    
#include <string>
#include <iostream>
#include "parser.hpp"

void display3DModel(const std::string& objFilePath);

#endif