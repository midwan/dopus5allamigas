/* */

address DOPUS.1
options results

do a = 1 to 30
	lister new
	handle = result
	command source handle devicelist
end
