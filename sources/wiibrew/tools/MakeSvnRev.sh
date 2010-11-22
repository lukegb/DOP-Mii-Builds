if [ ! -f ./include/svnrev.h ]; then
  REV=`svnversion -n ./`
  touch ./include/svnrev.h
  cat > ./include/svnrev.h <<EOF
#define SVN_REV $REV
#define SVN_REV_STR "$REV"
EOF
fi
