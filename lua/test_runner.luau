-- Simple Luau test runner for BlockEngine
-- Usage: BlockEngine --test

-- Test files are currently being injected by C++ as __TEST_FILES = { { path = "...", source = "..." }, ... }

local testsTotal, testsFailed = 0, 0
local assertsTotal, assertsFailed = 0, 0
local currentName = nil
local currentTestFailed = false

local function outputTest(ok, msg)
	print((ok and "[🟢 PASS] " or "[🟠 FAIL] ") .. msg)
end

local function expect(actual)
	local function fail(msg)
		assertsFailed = assertsFailed + 1
		currentTestFailed = true
		-- log assertion failure inline for context
		print("[ASSERT] " .. currentName .. ": " .. msg)
	end
	return {
		eq = function(expected)
			if actual ~= expected then
				fail("expected " .. tostring(expected) .. ", got " .. tostring(actual))
			else
				-- assertion passed
			end
            assertsTotal = assertsTotal + 1
		end,
		truthy = function()
			if not actual then
				fail("expected truthy, got " .. tostring(actual))
			else
				-- assertion passed
			end
            assertsTotal = assertsTotal + 1
		end,
		isA = function(className)
			if type(actual) == "userdata" and actual:IsA(className) then
				-- assertion passed
			else
				fail("expected userdata IsA(" .. tostring(className) .. ")")
			end
            assertsTotal = assertsTotal + 1
		end,
	}
end

local function test(name, fn)
	testsTotal = testsTotal + 1
	currentName = name
	currentTestFailed = false
	local ok, err = pcall(fn)
	if not ok then
		currentTestFailed = true
		print("[🔴 ERROR] " .. name .. ": runtime error: " .. tostring(err))
	end
	if currentTestFailed then
		testsFailed = testsFailed + 1
		outputTest(false, name)
	else
		outputTest(true, name)
	end
end

-- Make globals visible to test files
_G.test = test
_G.expect = expect

-- Discover and run tests
local entries = rawget(_G, "__TEST_FILES")
if type(entries) ~= "table" then
	print("No tests found in lua/tests/*.luau")
else
	for i = 1, #entries do
		local e = entries[i]
		local path = e and e.path or "(unknown)"
		local src = e and e.source or ""
		local ok, err = __RUN_CHUNK and __RUN_CHUNK(src, "@" .. path) or false, "no __RUN_CHUNK"
		if not ok then
			failed = failed + 1
			outputTest(false, path .. ": " .. tostring(err))
		end
	end
end

print("")
print(string.format("Tests: %d, Failed: %d", testsTotal, testsFailed))
print(string.format("Assertions: %d total, %d failed", assertsTotal, assertsFailed))

if testsFailed > 0 or assertsFailed > 0 then
	-- Non-zero exit to signal failure to CI
	error("TESTS_FAILED", 0)
end
