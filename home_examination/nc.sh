#!/bin/bash
valgrind --leak-check=full --track-origins=yes --log-file=broza ./client
