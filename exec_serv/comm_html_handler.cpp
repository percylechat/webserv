struct request{
    bool success;
    std::string response;
    int error_code;
    std::string error_msg;
}

void first_dispatch(char *msg){
    if (msg[0] == 'G' && msg[1] == 'E' && msg[2] == 'T')
        handle_get(msg);
    else if (msg[0] == 'D' && msg[1] == 'E')
        handle_delete(msg);
    else if (msg[0] == 'P' && msg[1] == 'O' && msg[2] == 'S' && msg[3] == 'T')
        handle_post(msg);
    else
        set_error(400);
}