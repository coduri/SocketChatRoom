cmake_minimum_required(VERSION 3.23)
project(SocketChatRoom_Client C)

set(CMAKE_C_STANDARD 11)

add_executable(SocketChatRoom_Client
        client.c
        ../libraries/stdoutUtil.h
        ../libraries/messageProtocol.h
        ../libraries/messagePrinting.h)
