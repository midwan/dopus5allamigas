/* */

address 'DOPUS.1'
options results

lister new 'S:'
handle = result
lister wait handle

command wait all
say result
command wait none
say result

lister select handle 'startup-sequence' 1
lister refresh handle
lister wait handle
command wait doubleclick
say result
