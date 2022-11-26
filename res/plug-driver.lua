env = {
    print = print,
    opus = {

    }
}

-- run code under environment [Lua 5.2]
local function run(untrusted_code)
    local untrusted_function, message = load(untrusted_code, nil, 't', env)

    if not untrusted_function then return nil, message end

    return pcall(untrusted_function)
end

function main(args)
    local res, message = run("myvar=10\nprint('Printing from runtime!')\nfunction opus.update(dt) print('update with val: '..dt) end")
    if not res then
        print(message)
    end

    if env.update ~= nil then
        env.update(16.7)
    end
end
