./mutex3  | grep '^\(read\|write\|mult\|print\|gen\)' | cut -d ':' -f 1 | sort | uniq -c
