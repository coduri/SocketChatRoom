cmake_minimum_required(VERSION 3.23)
project(SocketChatRoom_Server C)

set(CMAKE_C_STANDARD 11)

add_executable(SocketChatRoom_Server
        server.c
        ../libraries/stdoutUtil.h
        ../libraries/messageProtocol.h
        ../libraries/messagePrinting.h)
