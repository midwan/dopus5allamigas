/* */

address DOPUS.1
options results
options failat 11

dopus front

if ~show('l','rexxsupport.library') then
	call addlib('rexxsupport.library',0,-30,0)

call openport('test-handler')

/* set up menu item */
iconmenu.count=3
iconmenu.0='foo'
iconmenu.1='bar'
iconmenu.2='baz'

/* add an icon */
dopus addappicon 'test-handler' "'Test Icon'" 1 info snap close local menu iconmenu.
icon=result

say 'Icon added.. play with it. To remove, select Close from the popup menu'

flag = 0

/* loop around */
do while flag = 0
	call waitpkt('test-handler')

	packet=getpkt('test-handler')
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

	if arg0 = 'close' then flag = 1
	if arg0 = 'removed' then flag = 2
end

/* remove icon */
if flag~=2 then dopus remappicon icon
