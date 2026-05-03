/* */
address DOPUS.1
options results

lister new 'work:'
lister1 = result
lister wait lister1

lister new
lister2 = result

lister copy lister1 lister2
