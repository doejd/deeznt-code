--- Highligth Keywords
highlight("import", "reserved")
highlight("from", "reserved")
highlight("while", "reserved")
highlight("if", "reserved")
highlight("else", "reserved")
highlight("elif", "reserved")

highlight("False", "binary")
highlight("True", "binary")
highlight("None", "binary")

--- Arithmetic Operators
highlight("+", "operator")
highlight("-", "operator")
highlight("*", "operator")
highlight("/", "operator")
highlight("%", "operator")
highlight("**", "operator")
highlight("//", "operator")

--- Assignment Operators
highlight("=", "operator")
highlight("+=", "operator")
highlight("!=", "operator")
highlight("*=", "operator")
highlight("/=", "operator")
highlight("%=", "operator")
highlight("//=", "operator")
highlight("**=", "operator")
highlight("&=", "operator")
highlight("|=", "operator")
highlight("^=", "operator")
highlight(">>=", "operator")
highlight("<<=", "operator")

--- Comparison Operators
highlight("==", "operator")
highlight("!=", "operator")
highlight(">", "operator")
highlight("<", "operator")
highlight(">=", "operator")
highlight("<=", "operator")

--- Logical Operators
highlight("and", "reserved")
highlight("or", "reserved")
highlight("not", "reserved")

--- Membership Operators
highlight("in", "reserved")

--- Special Characters
highlight("{", "binary")
highlight("}", "binary")
highlight("[", "binary")
highlight("]", "binary")

--- Strings
highlight_region("'", "'", "string")
highlight_region('"', '"', "string")
highlight_region('"""', '"""', "string")

--- User Comments
highlight_region("#", "", "comments", true)
--- Autocomplete

function detect_functions(content)
    local functionNames = {}

    for line in content:gmatch("[^\r\n]+") do
        -- Match the "def" keyword for regular functions
        local functionName = line:match("def%s+([%w_]+)%s*%(")
        if functionName then
            table.insert(functionNames, functionName)
        end

        -- Match the "async def" pattern for asynchronous functions
        local asyncFunctionName = line:match("async%s+def%s+([%w_]+)%s*%(")
        if asyncFunctionName then
            table.insert(functionNames, asyncFunctionName)
        end
    end

    return functionNames
end



function detect_variables(content)
    local variable_names = {"self", "__name__", "__annotations__", "__build_class__", "__builtins__", "__cached__", "__dict__", "__doc__", "__file__", "__import__", "__loader__", "__name__", "__package__", "__path__", "__spec__"}
    local lines = content:gmatch("[^\r\n]+")

    for line in lines do
        -- Check for Python variable assignment syntax
        local assignment = line:match("(%w+)%s*=%s*.+")
        if assignment then
            local variable_name = assignment:match("(%w+)")
            table.insert(variable_names, variable_name)
        end
    end

    return variable_names
end

