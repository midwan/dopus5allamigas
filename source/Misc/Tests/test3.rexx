/* */

address DOPUS.1
options results

lf='0a'x
string=''

do i=1 to 40
	string=string||i '1234567890123456789012345678901234567890123456789012345678901234567890'lf
end

do i=40 to 100
	string=string||i 'testing...'lf
	dopus request '"'string'" ok|cancel'
	if rc=0 then
		leave
	end
