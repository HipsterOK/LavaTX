-- Test OpenTX functions
local function init()
  -- Test basic functions
  print("Testing OpenTX functions...")
  
  -- Test LCD functions
  if lcd then
    print("LCD functions available")
  else
    print("LCD functions NOT available")
  end
  
  -- Test __opentx
  if __opentx then
    print("__opentx available")
    if __opentx.CHAR_UP then
      print("CHAR_UP available")
    else
      print("CHAR_UP NOT available")
    end
  else
    print("__opentx NOT available")
  end
  
  -- Test crossfire functions
  if crossfireTelemetryPop then
    print("Crossfire functions available")
  else
    print("Crossfire functions NOT available")
  end
end

local function run(event, touchState)
  if event == nil then
    return 2
  end
  
  lcd.clear()
  lcd.drawText(10, 10, "OpenTX Test Loaded!", 0)
  lcd.drawText(10, 30, "Press EXIT to quit", 0)
  
  if event == EVT_VIRTUAL_EXIT then
    return 1
  end
  
  return 0
end

return { init=init, run=run }
