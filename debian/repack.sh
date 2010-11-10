#!/bin/sh
# tiny script to strip upstream Debian packaging from the upstream source tarball.

set -e

package_name=lipsia

CLOPTS=`getopt -o u: -l upstream-version: -n 'repack.sh' -- "$@"`

if [ $? != 0 ] ; then
  echo "Terminating..." >&2
  exit 1
fi

# Note the quotes around `$CLOPTS': they are essential!
eval set -- "$CLOPTS"

while true ; do
  case "$1" in
    -u|--upstream-version) shift; upstream_version=$1; shift;;
    --) shift ; break ;;
    *) echo "Internal error! ($1)"; exit 1;;
  esac
done

if [ $# -gt 1 ]; then
  printf "Too many arguments: $@.\n\n"
  exit 1
fi

if [ -z "$1" ]; then
  echo "No tarball given."
  exit 1
fi

if [ -z "$upstream_version" ]; then
  echo "Please provide the upstream version of this tarball."
  exit 1
fi

echo "Processing tarball '$1'"

curdir=$(pwd)
wdir=$(mktemp -d)

echo "Extracting tarball to '$wdir'"
tar --directory $wdir --exclude='.svn' -xzf $1

# determine upstream dir
updir=$(ls -1 $wdir)

if [ -z "$updir" ]; then
  echo "Cannot determine upstream source directory. Something is fishy."
  exit 1
else
  echo "Found directory '$updir'"
fi

# sanitize upstream dir
mv $wdir/$updir $wdir/${package_name}-${upstream_version}

echo "Repackaging tarball..."
tar --directory $wdir \
    --exclude='debian/*' --exclude='debian' --exclude='.svn' \
    --exclude='*~' --exclude='src/dictov' \
    -czf ${package_name}_$upstream_version.orig.tar.gz ${package_name}-$upstream_version

echo "Cleanup"
rm -rf $wdir
