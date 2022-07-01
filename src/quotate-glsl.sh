#!/bin/sh

# https://www.khronos.org/opengles/sdk/tools/Reference-Compiler/
#
# .vert - a vertex shader
# .tesc - a tessellation control shader
# .tese - a tessellation evaluation shader
# .geom - a geometry shader
# .frag - a fragment shader
# .comp - a compute shader

FILENAMES=$(find . -maxdepth 1 -regextype egrep -regex '.*(vert|tesc|tese|geom|frag|comp)')

CURRENT_GCC_VER="$(gcc -dumpversion)"
REQUIRED_GCC_VER="11.0.0"

for file in $FILENAMES
do
    # https://stackoverflow.com/questions/20443560/how-to-practically-ship-glsl-shaders-with-your-c-software
    awk 'NF { print "\""$0"\\""n""\""}' "$file" > "GLSL/$file.quoted"




    # use different quotation for different gcc version
    # see: https://unix.stackexchange.com/questions/285924/how-to-compare-a-programs-version-in-a-shell-script
    # author: Luciano Andress Martini
    if [ "$(printf '%s\n' "$REQUIRED_GCC_VER" "$CURRENT_GCC_VER" | sort -V | head -n1)" = "$REQUIRED_GCC_VER" ]; then 
        # "Greater than or equal to ${REQUIRED_GCC_VER}"
	echo '"\0"' >> "GLSL/$file.quoted"
    else
        # "Less than ${REQUIRED_GCC_VER}"
	echo '""\0"' >> "GLSL/$file.quoted"
    fi
done
