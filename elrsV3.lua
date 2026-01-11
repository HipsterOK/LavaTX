-- Optimized ELRS v3 script for TBS Tango (reduced memory usage)
local deviceId = 0xEE
local handsetId = 0xEF
local deviceName = ""
local lineIndex = 1
local pageOffset = 0
local edit = nil
local fieldPopup = nil
local fieldTimeout = 0
local loadQ = {}
local fieldData = {}
local fields = {}
local devices = {}
local goodBadPkt = "?/???    ?"
local elrsFlags = 0
local fields_count = 0
local backButtonId = 2
local exitButtonId = 3
local devicesRefreshTimeout = 50
local folderAccess = nil
local exitscript = 0

local COL2 = 70
local maxLineIndex = 6
local textXoffset = 0
local textYoffset = 3
local textSize = 8

-- Compact functions table
local functions = {
  [1] = { display = function(f,y,a) lcd.drawText(80,y,tostring(f.value or "N/A"),a) end }, -- UINT8
  [2] = { display = function(f,y,a) lcd.drawText(80,y,tostring(f.value or "N/A"),a) end }, -- INT8
  [3] = { display = function(f,y,a) lcd.drawText(80,y,tostring(f.value or "N/A"),a) end }, -- UINT16
  [4] = { display = function(f,y,a) lcd.drawText(80,y,tostring(f.value or "N/A"),a) end }, -- INT16
  [10] = { display = function(f,y,a) -- SELECT
    local val = f.values and f.values[f.value+1] or tostring(f.value or "N/A")
    lcd.drawText(80,y,val,a)
  end },
  [12] = { display = function(f,y,a) lcd.drawText(80,y,">>",a) end }, -- FOLDER
  [13] = { display = function(f,y,a) lcd.drawText(80,y,f.value or "INFO",a) end }, -- INFO
  [14] = { display = function(f,y,a) -- COMMAND
    local status = f.status == 0 and "Ready" or f.status == 1 and "Busy" or "Error"
    lcd.drawText(80,y,status,a)
  end },
  [15] = { display = function(f,y,a) lcd.drawText(80,y,"<<BACK",a) end }, -- back
  [16] = { display = function(f,y,a) lcd.drawText(80,y,"DEVICE",a) end }, -- device
  [17] = { display = function(f,y,a) lcd.drawText(80,y,"FOLDER",a) end }, -- deviceFOLDER
  [18] = { display = function(f,y,a) lcd.drawText(80,y,"EXIT",a) end } -- exit
}

local function allocateFields()
  fields = {}
  for i=1, fields_count + 2 do fields[i] = {} end
  backButtonId = fields_count + 2
  fields[backButtonId] = {name="----BACK----", parent=255, type=14}
  exitButtonId = backButtonId + 1
  fields[exitButtonId] = {id=exitButtonId, name="----EXIT----", type=17}
end

local function getField(line)
  local counter = 1
  for i=1,#fields do
    local field = fields[i]
    if not field.hidden then
      if counter == line then return field end
      counter = counter + 1
    end
  end
end

local function handleEvent(event)
  if event == EVT_VIRTUAL_NEXT then
    lineIndex = lineIndex + 1
    if lineIndex > maxLineIndex then
      lineIndex = maxLineIndex
      pageOffset = pageOffset + 1
    end
  elseif event == EVT_VIRTUAL_PREV then
    lineIndex = lineIndex - 1
    if lineIndex < 1 then
      lineIndex = 1
      if pageOffset > 0 then pageOffset = pageOffset - 1 end
    end
  elseif event == EVT_VIRTUAL_EXIT then
    exitscript = 1
  end
end

local function runDevicePage(event)
  handleEvent(event)
  
  lcd.clear()
  lcd.drawText(0, 0, "ExpressLRS", INVERS)
  lcd.drawText(0, textSize, goodBadPkt, 0)
  
  for y=1, maxLineIndex do
    local field = getField(pageOffset + y)
    if not field then break end
    
    local attr = (lineIndex == (pageOffset + y)) and INVERS or 0
    if field.name then
      lcd.drawText(textXoffset, y*textSize + textYoffset, field.name, 0)
    end
    if functions[field.type+1] and functions[field.type+1].display then
      functions[field.type+1].display(field, y*textSize + textYoffset, attr)
    end
  end
end

local function refreshData()
  -- Simplified data refresh without heavy Crossfire operations
  local time = getTime()
  if time > devicesRefreshTimeout and fields_count < 1 then
    devicesRefreshTimeout = time + 100
    -- Simplified device discovery
    goodBadPkt = "0/0      -"
  end
end

local function init()
  allocateFields()
  goodBadPkt = "No signal"
  deviceName = "ELRS TX"
end

local function run(event, touchState)
  if event == nil then
    error("Cannot be run as a model script!")
    return 2
  end

  refreshData()

  if fieldPopup ~= nil then
    if event == EVT_VIRTUAL_EXIT then fieldPopup = nil end
    lcd.clear()
    lcd.drawText(10, 20, "Command running...", 0)
  else
    runDevicePage(event)
  end

  return exitscript
end

return { init=init, run=run }
