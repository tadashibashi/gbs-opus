-- Each C++ plugin object will contain a lua context, using this sandbox driver
-- Load the plugin module via run(code_string)
-- User will

-- Environment variables to pass into the sandbox
env = {
    math = math,
    require = require,
    print = print,
    time = {
        delta = nil, --
        sinceStart = nil
    }
}

-- Load the plugin module into "env". Then you can run any code using e.g. env.update(10)
local function run(untrustedCode)
    local untrustedFunction, message = load(untrustedCode, nil, 't', env)

    if not untrustedFunction then return nil, message end

    return pcall(untrustedFunction)
end

-- Update callback, gets channel
local function update(chan)
    if env.update ~= nil then
        env.update(chan)
    end
end

function test(args)
    local res, message = run("myvar=10\nprint('Printing from runtime!')\nfunction update(dt) print('update with val: '..dt) myvar = myvar + 1 end\n"..
        "require('scripts.yo')\nprint(math.ceil(132.123123123))")
    env.update(10)
    if not res then
        print(message)
    end
end

test({"hi", "yo"});
