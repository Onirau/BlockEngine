-- Tests for Parts

test("Part Name Comparison", function()
	local p = Instance.new("Part")
	p.Name = "BlockA"
	p.Color = Color3.new(1, 0, 0)
	expect(p.Name == "BlockA").truthy()
end)

test("Create Part and Set Properties", function()
	local p = Instance.new("Part")
	p.Name = "BlockA"
	p.Color = Color3.new(1, 0, 0)
	expect(p:IsA("Instance")).truthy()
	expect(p:IsA("Part")).truthy()
end)

test("Userdata Part Parent Comparison", function()
	local parent = Instance.new("Part")
	local child = Instance.new("Part")
	child.Parent = parent
	expect(child.Parent == parent).eq(true)
end)
