#!/bin/bash
protoc -I=. --cpp_out=. ./InteractiveProtocol.proto
protoc -I=. --cpp_out=. ./InteractiveProtocolManagement.proto
