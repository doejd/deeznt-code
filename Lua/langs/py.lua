--- Highligth Keywords
highlight("import", "reserved")
highlight("from", "reserved")
highlight("while", "reserved")
highlight("if", "reserved")
highlight("else", "reserved")
highlight("elif", "reserved")
highlight("as", "reserved")
highlight("class", "reserved")
highlight("return", "reserved")
highlight("def", "reserved")
highlight("for", "reserved")
highlight("pass", "reserved")
highlight("try", "reserved")
highlight("except", "reserved")
highlight("yield", "reserved")

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

function detect_functions(content)
    local functionNames = {}

    for line in content:gmatch("[^\r\n]+") do
        local functionName = line:match("def%s+([%w_]+)%s*%(")
        if functionName then
            table.insert(functionNames, functionName)
        end
        local asyncFunctionName = line:match("async%s+def%s+([%w_]+)%s*%(")
        if asyncFunctionName then
            table.insert(functionNames, asyncFunctionName)
        end
    end

    return functionNames
end

function detect_variables(content)
    local variable_names = {"self", "__name__", "__annotations__", "__build_class__", "__builtins__", "__cached__", "__dict__", "__doc__", "__file__", "__import__", "__loader__", "__name__", "__package__", "__path__", "__spec__"}

    for line in content:gmatch("[^\r\n]+") do
        local assignment = line:match("^%s*([%w_%.]+)%s*=%s*[^=].*")
        if assignment then
            table.insert(variable_names, assignment)
        end
    end
    return variable_names
end

function detect_imports(content)
    local import_names = {}
    for line in content:gmatch("[^\r\n]+") do
        local import_as = line:match("^%s*import%s+[%w_%.]+%s+as%s+([%w_]+)")
        if import_as then
            table.insert(import_names, import_as)
        end

        local from_import_as = line:match("^%s*from%s+[%w_%.]+%s+import%s+[%w_%.]+%s+as%s+([%w_]+)")
        if from_import_as then
            table.insert(import_names, from_import_as)
        end

        local import_simple = line:match("^%s*import%s+([%w_,%s]+)")
        if import_simple then
            for name in import_simple:gmatch("([%w_]+)") do
                table.insert(import_names, name)
            end
        end

        local from_import = line:match("^%s*from%s+[%w_%.]+%s+import%s+([%w_]+)")
        if from_import then
            table.insert(import_names, from_import)
        end
    end
    
    return import_names
end