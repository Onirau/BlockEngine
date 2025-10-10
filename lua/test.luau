-- can we make parts
local part1 = Instance.new("Part")
part1.Color = Color3.new(0,0,1)
part1.Position = Vector3.new(0, 2.5, 0)
part1.Size = Vector3.one * 2
part1.Shape = "CornerWedge"
part1.Parent = workspace

local part2 = Instance.new("Part")
part2.Color = Color3.new(0,1,0)
part2.Position = Vector3.new(2.5, 2.5, 0)
part2.Size = Vector3.one * 2
part2.Shape = "Block"
part2.Parent = part1

-- try making a lua source
local script = Instance.new("Script")
script.Name = "MyScript"
script.Source = "print('Hello from script!')"
script.Enabled = true
-- script:Execute()

-- test debug
print(part1:IsA("Object")) -- expect true
print(part1:IsA("Instance")) -- expect true
print(part1:IsA("Part")) -- expect true

print(part2:IsA("Object")) -- expect true
print(part2:IsA("Instance")) -- expect true
print(part2:IsA("Part")) -- expect true

print(part2.Parent == part1) -- expect true

print(script.Parent) -- expect nil
print(script:IsA("Script")) -- expect true