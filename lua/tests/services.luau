-- Tests for Parts

test("Require Exists", function()
	expect(require).defined()
end)

test("Workspace Exists", function()
	expect(workspace).defined()
end)

test("Game Exists", function()
	expect(game).defined()
end)

test("Game.GetService Exists", function()
	expect(game.GetService).defined()
end)

test("Game:GetService Workspace to workspace Comparison", function()
	expect(workspace == game:GetService("Workspace")).truthy()
end)