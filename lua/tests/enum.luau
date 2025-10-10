test("Cant Redefine Enum tables", function()
    expect(function() Enum.PartType = "k" end).throws()
    expect(Enum.PartType.Block).defined()
end)