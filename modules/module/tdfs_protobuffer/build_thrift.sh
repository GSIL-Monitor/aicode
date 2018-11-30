#!/bin/bash
rm -fr gen-cpp
thrift -r -v -gen cpp Trans.thrift

