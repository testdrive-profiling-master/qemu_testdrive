local sRun = String(exec("ls src/*"))
sRun:Replace("\r", "", true)
sRun:Replace("\n", " ", true)
sRun:Replace("  ", " ", true)
sRun:Trim(" ")

local f			= TextFile()
local bMatch	= false

if f:Open(".qemu/testdrive_src_list.txt") then
	local sContents = f:GetAll()
	bMatch = (sContents == sRun.s)
	f:Close()
end

if bMatch == false then
	f:Create(".qemu/testdrive_src_list.txt")
	f:Put(sRun.s)
	f:Close()
	
	exec("touch .qemu/meson.build")
end
