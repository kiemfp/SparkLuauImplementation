local myTable = getgenv()

-- Loop through each key-value pair in the table
for key, value in pairs(myTable) do
    -- Print the key and the corresponding value
    -- 'tostring(value)' is used to ensure that even non-string values can be printed without error.
    print("Key: " .. tostring(key) .. ", Value: " .. tostring(value))
end
