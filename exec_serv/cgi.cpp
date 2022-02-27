#include "cgi.hpp"

std::map<std::string, std::string> create_env(Bundle_for_response bfr, serverConf conf){
    std::map<std::string, std::string> cgi;
    cgi.insert(std::make_pair("AUTH_TYPE", ""));
    cgi.insert(std::make_pair("REMOTE_HOST", ""));
    cgi.insert(std::make_pair("REMOTE_IDENT", ""));
    cgi.insert(std::make_pair("REMOTE_USER", ""));
    cgi.insert(std::make_pair("GATEWAY_INTERFACE", "CGI/1.1"));
    cgi.insert(std::make_pair("SERVER_PROTOCOL", "HTTP/1.1"));
    cgi.insert(std::make_pair("SERVER_SOFTWARE", "webserv/1.1"));
    cgi.insert(std::make_pair("REMOTE_ADDR", "127.0.0.1"));
    cgi.insert(std::make_pair("SERVER_PORT", conf.http.data()[bfr.specs]["server"]["listen"][0]));
    cgi.insert(std::make_pair("CONTENT_TYPE", bfr.re.content_type));
    cgi.insert(std::make_pair("REQUEST_METHOD", bfr.re.type));
    if (bfr.re.type == "GET")
        cgi.insert(std::make_pair("CONTENT_LENGTH", "0"));
    else {
        int t = static_cast <int>(bfr.re.body.size());
        std::stringstream ss;
        ss << t;
        std::string s = ss.str();
        cgi.insert(std::make_pair("CONTENT_LENGTH", s));
        cgi.insert(std::make_pair("BODY", bfr.re.body));
    }
    cgi.insert(std::make_pair("PATH_INFO", bfr.re.page));
    cgi.insert(std::make_pair("PATH_TRANSLATED", bfr.re.page));
    cgi.insert(std::make_pair("QUERY_STRING", bfr.re.query));
	// cgi.insert(std::make_pair("DIR_PATH"], _loc.get_root();
//Contient le chemin HTTP du script les données transmises dans l'appel.
//Supposons que le script a l'adresse http://ma.page.net/cgi-bin/test.pl et qu'il a été appelé avec http://ma.page.net/cgi-bin/test.pl?User=Serge.
//Alors la variable REQUEST_URI livre la valeur /cgi-bin/test.pl?User=Serge.
	// _env["REQUEST_URI"]			= _loc.get_uri();
	// _env["SCRIPT_NAME"]			= _loc.get_cgi_path();
	// _env["SERVER_NAME"]			= _conf.get_server_name();

	// _env["REDIRECT_STATUS"]		= "200";
    return cgi;
}

char **go_env(std::map<std::string, std::string> cgi){
    char **env = new char*[cgi.size() + 1];
    unsigned int i = 0;
    std::map<std::string, std::string>::iterator it = cgi.begin();
    while (it != cgi.end())
    {
        std::string tmp = it->first;
        tmp.append("=");
        tmp.append(it->second);
        env[i] = strdup(tmp.c_str());
        tmp.clear();
        it++;
        i++;
    }
    env[i] = NULL;
    return env;
}

std::string handle_cgi(Bundle_for_response bfr, serverConf conf){
// TO DO be careful of script permissions
    if (bfr.re.type == "DELETE"){
        bfr.re.error_type = 405;
        return go_error(bfr.re.error_type, conf, bfr);
    }
    bfr.re.page = bfr.re.page.substr(1, bfr.re.page.size() - 1);
    bfr.re.page = bfr.re.page.substr(bfr.re.page.find_last_of("/") + 1, bfr.re.page.size() - bfr.re.page.find_last_of("/") + 1);
    std::map<std::string, std::string> cgi = create_env(bfr, conf);
    int pipe_fd[2];
    int fd_save[2];
    pid_t pid;

    fd_save[0] = dup(STDIN_FILENO);
    fd_save[1] = dup(STDOUT_FILENO);
    std::string name = "cgi_output";
    if (pipe(pipe_fd))
        exit(EXIT_FAILURE);
    pid = fork();
    if (pid == -1)
        return "";
	else if (pid == 0){
        char *args[2];
        char **env = go_env(cgi);
                // std::cout << "ping" << conf.http.data()[bfr.specs]["server"]["root"][0].c_str() << std::endl;
        std::string te = conf.http.data()[bfr.specs]["server"]["root"][bfr.root];
        chdir(te.c_str());
        std::cout << "ping" << bfr.re.page.c_str() << std::endl;
		args[0] = (char*)bfr.re.page.c_str();
		args[1] = NULL;

		close(pipe_fd[1]);
		dup2(pipe_fd[0], 0);
		int	fd_tmp = open(name.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
		if (fd_tmp < 0){
            std::cout << "sortie1" << std::endl;
            
			return "";
        }
        std::cerr << "pid 0" << std::endl;
		dup2(fd_tmp, 1);
        // dup2(fd_tmp, pipe_fd);
		if (execve(args[0], args, env) == -1){
            std::cerr << "sortie2" << errno << std::endl;
            delete [] env;
            close(0);
		    close(fd_tmp);
		    close(pipe_fd[0]);
			return "";
        }
		close(0);
		close(fd_tmp);
        std::cerr << "im ok" << std::endl;
		close(pipe_fd[0]);
		exit(0);
	}
	else{
        std::cerr << "pid 1" << std::endl;
		close(pipe_fd[0]);
		if (write(pipe_fd[1], bfr.re.body.c_str(), bfr.re.body.length()) < 0)
			std::cerr << "Writing failed in CGI" << std::endl;
		close(pipe_fd[1]);
                    // delete [] env;
		waitpid(pid, NULL, 0);
	}
	dup2(fd_save[0], STDIN_FILENO);
	dup2(fd_save[1], STDOUT_FILENO);
	close(fd_save[0]);
	close(fd_save[1]);
    std::cerr << "end" << std::endl;
	if (pid == 0)
		exit(0);
//TO DO file name + path
    std::basic_ifstream<char> fin("cgi/cgi_output");
	std::ostringstream oss;
	oss << fin.rdbuf();
	std::string ret(oss.str());
    std::string end = "HTTP/1.1 200 OK \r\nContent-Length: " ;
    std::stringstream ss;
    ss << ret.size();
    end.append(ss.str());
    end.append("\r\n\r\n" + ret);
    std::cout << "ok" << ret << std::endl;
	return end;
}