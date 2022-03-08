-- Unit testing script
local silent = false -- False by default. Dont suppress output.

if arg[1] == "--silent" then
  silent = true
  table.remove(arg, 1)
end

EXECUTABLE_PATH = "rfscss"

local rprint = function(prefix, str, id)
  io.stdout:write(prefix:format(id) .. ' ')
  print(str)
  return str
end

-- Get all files in directory

local check_output = function(output, expected)
  local captured, total = output:match("captured (%d+) of (%d+) selectors")
  captured, total = tonumber(captured), tonumber(total)

  if expected[1] == 0 or expected[2] == 0 or (captured == expected[1] and total == expected[2]) then
    return true
  end

  return false, captured, total
end

local rfscss = function(test_num, test, params, expected)
    params = "--enable-warnings " .. (params or "")
    output = io.popen(
      rprint(
        "[test %d]",
        ("%s ./%s %s"):format(EXECUTABLE_PATH, test, params),
        test_num
      )
    ):read("*a")

    print(
      ("%s\n%s"):format(('-'):rep(10), output)
    )

    return check_output(output, expected)
end

local check_extracted = function()
  return {
    ['at-rules.scss'] = os.remove('at-rules.scss') or false,
    ['capturename.scss'] = os.remove('capturename.scss') or false,
    ['imports.scss'] = os.remove('imports.scss') or false,
    ['interpolated.scss'] = os.remove('interpolated.scss') or false,
    ['literal.scss'] = os.remove('literal.scss') or false,
    ['variables.scss'] = os.remove('variables.scss') or false
  }
end

local args = {
    {
      params = "--list rule-list.txt",
      test = "test_file.scss",
      expected = {
        0, 0, "rule-list.txt"
      },

      check = function()
        return os.remove("rule-list.txt")
      end
    },
    {
      params = "--tidy",
      test = "test_file.scss",
      expected = {
        10, 11, "tidy-not__valid__filename.scss"
      },

      check = function()
        return os.remove("tidy-not__valid__filename.scss")
      end
    },
    {
      params = "--inline-rfscss \".?-selector->?.scss\"",
      test = "test_file.scss",
      expected = {
        1, 11, "unlisted.scss"
      },

      check = function()
        return os.remove("unlisted.scss")
      end
    }
}

if #args ~= 0 then
    for i, arg in ipairs(args) do
        local detail = ""
        local success, captured, total = rfscss(i, arg.test, arg.params, arg.expected)
        if success then
          if arg.check then
            success = arg.check()
            if not success then
              detail = (
                "Test check failed. Output file expected was not found.\n" ..
                  "Output file: %s\n"
              ):format(arg.expected[3])
            end
          end
        else
          detail = (
              "Expectations weren't fulfilled by the test.\n" ..
              "Captured %s (expected %s)\n" ..
              "Found %s (expected %s)"
          ):format(captured or "?", arg.expected[1], total or "?", arg.expected[2])
        end

        if not success then
          print("[test " .. i .. "] FAILED")
          print(detail)
          os.exit(i)
        end
    end
else
  print("No testing was done.")
end

local failed = false
for file, is_found in pairs(check_extracted()) do
  if not is_found then
    print(
      ("[run-tests.lua] %s expected but is missing."):format(file)
    )
    failed = true
  end
end

if failed then
  print("[run-tests.lua] FAILED.")
  print("Not all of the expected output files were found.")
else
  print("[run-tests.lua] All build tests passed successfully.")
  os.execute("rm -rf *.scss *.txt")
end