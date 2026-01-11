-- Simple ELRS tool for TBS Tango
local deviceId = 0xEE
local lineIndex = 1
local pageOffset = 0
local edit = nil
local exitscript = 0

local COL2 = 70
local maxLineIndex = 6
local textXoffset = 0
local textYoffset = 3
local textSize = 8

-- Simple menu items
local menuItems = {
  "Device Info",
  "WiFi Settings", 
  "Binding",
  "Power Levels",
  "Packet Rate",
  "Telemetry"
}

local function getField(line)
  return menuItems[line] or ""
end

local function handleEvent(event)
  if event == EVT_VIRTUAL_NEXT then
    lineIndex = lineIndex + 1
    if lineIndex > #menuItems then
      lineIndex = #menuItems
    end
  elseif event == EVT_VIRTUAL_PREV then
    lineIndex = lineIndex - 1
    if lineIndex < 1 then
      lineIndex = 1
    end
  elseif event == EVT_VIRTUAL_EXIT then
    exitscript = 1
  end
end

local function runDevicePage(event)
  handleEvent(event)
  
  lcd.clear()
  lcd.drawText(0, 0, "ExpressLRS TBS", INVERS)
  lcd.drawText(0, textSize, "No Crossfire Module", 0)
  lcd.drawText(0, textSize * 2, "Available Options:", 0)
  
  for y = 1, maxLineIndex do
    local item = getField(y)
    if item ~= "" then
      local attr = (lineIndex == y) and INVERS or 0
      lcd.drawText(textXoffset, y * textSize + textYoffset + textSize, item, attr)
    end
  end
end

local function init()
  return "ExpressLRS TBS"
end

local function run(event, touchState)
  if event == nil then return 2 end

  if event ~= 0 then
    runDevicePage(event)
  end

  return exitscript
end

return { init=init, run=run }
