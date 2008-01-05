#!/bin/sh
#
        for i in `ls`
        do
        if [ -f $i ]
        then
            if [ -n "`tail -1c $i`" ]
            then
                echo "Bearbeite $i"
                echo >> $i
            fi        
        fi
        done
