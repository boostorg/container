#!/bin/bash
# Copies every g23/flag script into /tmp (stripping CRLF) and runs the one
# named by $1.  /tmp is wiped between wsl invocations, so all staging must
# happen inside a single script.
set -u
G=/mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g23/flag
mkdir -p /tmp/g23
for f in "$G"/*.sh "$G"/*.py; do
   [ -f "$f" ] || continue
   tr -d '\r' < "$f" > "/tmp/g23/$(basename "$f")"
done
exec bash "/tmp/g23/$1" "${@:2}"
