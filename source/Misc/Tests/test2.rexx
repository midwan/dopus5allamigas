/* */

address DOPUS.1
options results

lister query source
handle=result
lister new
other=result

lister query handle selfiles stem 'test.'

do a = 0 to test.count - 1

	lister query handle entry test.a stem fileinfo.
/*
	fileinfo.display = a || "  " || fileinfo.name
*/
	fileinfo.filetype = fileinfo.filetype
	lister addstem other fileinfo.
/*
	say "name : "fileinfo.name
	say "size : "fileinfo.size
	say "type : "fileinfo.type
	say "sel  : "fileinfo.selected
	say "date : "fileinfo.date    fileinfo.datestring
	say "       "fileinfo.datenum||"."||fileinfo.time
	say "prot : "fileinfo.protect    fileinfo.protstring
	say "comm : "fileinfo.comment
	say "type : "fileinfo.filetype
	say "ver  : "fileinfo.version||"."||fileinfo.revision
	say "       "fileinfo.verdate
	say
*/
end

lister refresh other full
