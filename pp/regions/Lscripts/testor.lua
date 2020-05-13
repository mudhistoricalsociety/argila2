--[[
table[1] is time_info.hour (IG hour of the day)
table[2] is time_info.day (IG day of the month)
table[3] is sector_type for the room

	
source is who/what triggers the script
    ROOM    0
    OBJECT  1
    MOBILE  2

me is the rnum/vnum of the source

room is the rnum of the room with the trigger

math.random(lower, upper) generates a pretty good random number between lower and upper
--]]


function room_timer(table, source, me, room)
	
	rand = math.random(0, 100)

	pmess = ""
	
	--[[ 
	For City sector, generates a couple messages 
	--]]
	
	if (table[3] == 0) then
		if ((rand >= 1) and (rand <= 39)) then
			pmess = "A wagon rumbles past you.\n"
		elseif ((rand >= 40) and (rand <= 60)) then
			pmess = "A bunch of children chase each other as they run down the street.\n"		
		end
	end

	
	command = "echo"

   return command, pmess
   
end
--------------------------------------------------------------------
function obj_timer(table, source, me, room)

	rand = math.random(0, 100)

	--[[ 
	Between hours of midnight and 6am generates a message 
	--]]
	
	if (table[2] >= 0 and table[2] <=6 and rand <= 20) then
		pmess = "The fire crackles in the fireplace\n"
	end
	
	
	command = "echo"

   return command, fmess
end

-------------------------------------------------------------
function mob_timer(table, source, me, room)

	rand = math.random(0, 100)

	--[[
	Loads mvnum 1998, into the same room as the trigger, 10% of the time it is triggered. 
	--]]

	if ((rand >= 1) and (rand <= 10)) then
			pmess =  "1998 " .. room
	end
	
     command = "r_load"

     return command, pmess
end

