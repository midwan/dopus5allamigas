/* */

address DOPUS.1
options results
options failat 11

parse arg handler

if handler='' then
	say 'I need a name for the handler'
else do
	dopus front

	if ~show('l','rexxsupport.library') then
		call addlib('rexxsupport.library',0,-30,0)

	call openport(handler)

	/* add an icon */
	dopus addappicon handler "'Test Icon'" 1
	icon=result

	say 'Icon added.. play with it. To remove, drop DirectoryOpus on it'

	/* loop around */
	do while ((right(arg2,13) ~= 'DirectoryOpus') & (arg0 ~= 'removed'))
		call waitpkt(handler)

		packet=getpkt(handler)
		arg0=getarg(packet,0)
		arg1=getarg(packet,1)
		arg2=getarg(packet,2)
		arg3=getarg(packet,3)
		arg4=getarg(packet,4)

		say 'Arg0:' arg0
		say 'Arg1:' arg1
		say 'Arg2:' arg2
		say 'Arg3:' arg3
		say 'Arg4:' arg4

		call reply(packet,0)
	end

	/* remove icon */
	if arg0 ~= 'removed' then
		dopus remappicon icon

end
