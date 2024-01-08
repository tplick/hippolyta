#!/bin/sh

git archive --format tar HEAD . --prefix hippolyta-$1-src/ | gzip -9 > hippolyta-$1-src.tar.gz

