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

-- update qemu.ini
do
	local sEnv			= String()
	local sQEMU_ini		= String()
	local sTimeStamp	= tostring(math.floor(os.time() / (60*60*24)))
	sQEMU_ini:GetEnvironment("TESTDRIVE_PROFILE")
	sQEMU_ini = "@QEMU@" .. sQEMU_ini.s .. "Common/bin/qemu.ini"	-- QEMU configuration file

	sEnv:GetEnvironment("DATE" .. sQEMU_ini)
	
	if sEnv.s ~= sTimeStamp then
		local sCurCommit	= String()
		sCurCommit.s = exec("git ls-remote https://github.com/testdrive-profiling-master/qemu_testdrive.git HEAD")
		sCurCommit:Trim("\r\n\t ")
		if sCurCommit:CompareBack("HEAD") then
			sCurCommit:CutBack("HEAD")
			sCurCommit:Trim("\t ")
			
			-- commit check
			local sPrevCommit = String()
			sPrevCommit:GetEnvironment("COMMIT" .. sQEMU_ini)
			
			if (#sCurCommit.s == 40) and (sPrevCommit.s ~= sCurCommit.s) then
				sCurCommit:SetEnvironment("COMMIT" .. sQEMU_ini)
			end
			sEnv.s = sTimeStamp
			sEnv:SetEnvironment("DATE" .. sQEMU_ini)
		end
	end
end