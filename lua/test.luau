local lighting = game:GetService("Lighting")
print(lighting:GetSunDirection())

local baseplate = Instance.new("Part");
baseplate.Color = Color3.new(0.35,0.35,0.35)
baseplate.Position = Vector3.new(0, -8, 0)
baseplate.Size = Vector3.new(2048, 16, 2048)
baseplate.Name = "Baseplate"
baseplate.Parent = workspace

-- -- can we make parts
local spacing = 0
do
    local part = Instance.new("Part")
    part.Color = Color3.new(0,0,1)
    part.Position = Vector3.new(spacing, 2.5, 0)
    part.Size = Vector3.one * 2
    part.Shape = Enum.PartType.Block
    part.Parent = workspace
    spacing += 2.5
end

do
    local part = Instance.new("Part")
    part.Color = Color3.new(0,0,1)
    part.Position = Vector3.new(spacing, 2.5, 0)
    part.Size = Vector3.one * 2
    part.Shape = Enum.PartType.Ball
    part.Parent = workspace
    spacing += 2.5
end

do
    local part = Instance.new("Part")
    part.Color = Color3.new(0,0,1)
    part.Position = Vector3.new(spacing, 1.5, 0) -- TODO: gotta fix cylinder mesh
    part.Size = Vector3.one * 2
    part.Shape = Enum.PartType.Cylinder
    part.Parent = workspace
    spacing += 2.5
end

do
    local part = Instance.new("Part")
    part.Color = Color3.new(0,0,1)
    part.Position = Vector3.new(spacing, 2.5, 0)
    part.Size = Vector3.one * 2
    part.Shape = Enum.PartType.Wedge
    part.Parent = workspace
    spacing += 2.5
end

do
    local part = Instance.new("Part")
    part.Color = Color3.new(0,0,1)
    part.Position = Vector3.new(spacing, 2.5, 0)
    part.Size = Vector3.one * 2
    part.Shape = Enum.PartType.CornerWedge
    part.Parent = workspace
    spacing += 2.5
end


-- -- try making a lua source
-- local script = Instance.new("Script")
-- script.Name = "TestScript"
-- script.Source = "print('Hello from script!')"
-- script.Enabled = true


-- local script2 = Instance.new("ModuleScript")
-- script2.Name = "TestModuleScript"
-- script2.Source = "return true"
-- script2.Enabled = true
-- -- script:Execute()

-- -- test debug
-- print(part1:IsA("Object")) -- expect true
-- print(part1:IsA("Instance")) -- expect true
-- print(part1:IsA("Part")) -- expect true

-- print(part2:IsA("Object")) -- expect true
-- print(part2:IsA("Instance")) -- expect true
-- print(part2:IsA("Part")) -- expect true

-- print(part2.Parent == part1) -- expect true

-- print(script.Parent) -- expect nil
-- print(script:IsA("Script")) -- expect true