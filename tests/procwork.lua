#!/usr/bin/lua

if arg[ 1 ] == "1" then
  io.stdout:write( "hello world\n" )
elseif arg[ 1 ] == "2" then
  io.stderr:write( "hello world\n" )
elseif arg[ 1 ] == "3" then
  io.stdout:write( "hello world\n" )
  io.stderr:write( "hello world\n" )
elseif arg[ 1 ] == "4" then
  error( "an error message" )
elseif arg[ 1 ] == "5" then
  -- no input/output!
elseif arg[ 1 ] == "6" then
  local l = io.read()
  io.write( l, "\n" )
elseif arg[ 1 ] == "7" then
  for l in io.lines() do
    io.write( l, "\n" )
  end
elseif arg[ 1 ] == "8" then
	local count = 4
	while count>0 do
		count=count-1
		io.write(count,"\n")
		io.flush ()
		require'socket'.sleep(2)
	end
end

