#include "../parsing_conf.hpp"
#include "request.hpp"

#include <iostream>
#include <sstream>
#include <fstream>
#include <istream>

#include <sys/types.h>
#include <sys/wait.h>

#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>

#include <errno.h>

std::string handle_cgi(Bundle_for_response bfr, serverConf conf);