from datetime import datetime
from subprocess import call
import ctypes
import sys

def RepairReplayLog(replayLog):
    isValid = False
    data = []
    dataSize = len(data)
    with open(replayLog) as f:
        for line in f:
            data.append(line)
        if "resultData" in line:
    	    isValid = True
    if not isValid:
	    data.append("], \"resultData\":{\"hasWin\":false,\"resultPayload\":\"AIBot Crashed Detected!\"}}")

    with open(replayLog, "w+") as f:
        f.truncate(0)
        for line in data:
            f.write(line)

if len(sys.argv) != 8:
    print("Invalid arguments")
    print(sys.argv)
    sys.exit(0)

mapname = sys.argv[1]
AIBOOTCAMP2_ENGINE_DIR = sys.argv[2]
AIBOT_PATH = sys.argv[3]
TEAM = sys.argv[4]
ConnectToProcessDelay = sys.argv[5]
InitTime = sys.argv[6]
TurnTime = sys.argv[7]

now = datetime.now()

replayfile=now.strftime("%Y%m%d_%H%M%S")

print("")
print("You are about to play a local match with the following parameters:")
print("")
print("   mapname                = [" + mapname + "]")
print("   AIBOOTCAMP2_ENGINE_DIR = [" + AIBOOTCAMP2_ENGINE_DIR + "]")
print("   AIBOT_PATH             = [" + AIBOT_PATH + "]")
print("   TEAM                   = [" + TEAM + "]")
print("   ConnectToProcessDelay  = [" + ConnectToProcessDelay + "]")
print("   InitTime               = [" + InitTime + "]")
print("   TurnTime               = [" + TurnTime + "]")
print("   replayfile             = [" + replayfile + "]")
print("")
input("Press Enter to continue...")
print("")

result = call([AIBOOTCAMP2_ENGINE_DIR+"/AIBootCamp2.exe", "-dllpath", AIBOT_PATH, "-mode", "match", "-scene", mapname, "-team", TEAM, "-replayfile", replayfile, "-connecttoprocessdelay", ConnectToProcessDelay, "-initdelay", InitTime, "-turndelay", TurnTime, "-quit", "-batchmode"])    
result = ctypes.c_int32(result).value

if result == 0:
  	print("Match Completed : Victory!")
elif result == -1 or result == 3:
  	print("Match Completed : Failure")
  	RepairReplayLog(".\\Replays\\" + replayfile + "\\" + mapname + "_" + replayfile + ".replay")
elif result == -2:
    print("Invalid Map Name [" + mapname + "]")