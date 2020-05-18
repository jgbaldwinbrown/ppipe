#!/bin/bash
set -e

./mutex${1} | grep -v '^[(a-z]' | sort -k1,1n
