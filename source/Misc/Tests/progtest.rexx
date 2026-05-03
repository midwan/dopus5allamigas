/* */

address DOPUS.1
options results

lister new
handle = result

lister set handle progress 500 'test progress'
lister set handle busy on
lister clear handle abort

do a = 1 to 500

	lister set handle progress count a
	lister set handle progress name a
	lister query handle abort

	if result = 1 then do
		a = 500
		lister clear handle abort
	end
end

lister clear handle progress
lister set handle progress 1200 'version 2'
lister clear handle abort

do a = 1 to 1200

	lister set handle progress count a
	lister set handle progress name a
	lister query handle abort

	if result = 1 then a = 1200
end

lister clear handle progress
lister set handle busy off
