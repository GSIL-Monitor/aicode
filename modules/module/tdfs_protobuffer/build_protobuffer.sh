#!/bin/bash
./protoc -I=. --cpp_out=. ./TDFS.proto
yes|cp *.h ../tdfs_sync/src
yes|cp *.cc ../tdfs_sync/src
yes|cp *.h ../tdfs_center/src
yes|cp *.cc ../tdfs_center/src

