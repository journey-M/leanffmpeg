import lldb
import os


print ('this is a test in lldb ')

exe = "./a"
debugger = lldb.SBDebugger.Create()
print (debugger)

# When we step or continue, don't return from the function until the process
# stops. Otherwise we would have to handle the process events ourselves which, while doable is
#a little tricky.  We do this by setting the async mode to false.

debugger.SetAsync (False)
target = debugger.CreateTarget(exe)
print (target)
#debugger.assertTrue(target, VALID_TARGET)
breakpoint = target.BreakpointCreateByName('main', 'a.out')

target = debugger.CreateTargetWithFileAndArch (exe, lldb.LLDB_ARCH_DEFAULT)
print (target)

process = target.LaunchSimple(None, None, os.getcwd())


if target:
    # If the target is valid set a breakpoint at main
    main_bp = target.BreakpointCreateByName ("main", target.GetExecutable().GetFilename());

    print (main_bp)

    # Launch the process. Since we specified synchronous mode, we won't return
    # from this function until we hit the breakpoint at main
    process = target.LaunchSimple (None, None, os.getcwd())

    # Make sure the launch went ok
    if process:
        # Print some simple process info
        state = process.GetState ()
        print (process)
        if state == lldb.eStateStopped:
            # Get the first thread
            thread = process.GetThreadAtIndex (0)
            if thread:
                # Print some simple thread info
                print (thread)
                # Get the first frame
                frame = thread.GetFrameAtIndex (0)
                if frame:
                    # Print some simple frame info
                    print (frame)
                    function = frame.GetFunction()
                    # See if we have debug info (a function)
                    if function:
                        # We do have a function, print some info for the function
                        print (function)
                        # Now get all instructions for this function and print them
                        insts = function.GetInstructions(target)
                        disassemble_instructions (insts)
                    else:
                        # See if we have a symbol in the symbol table for where we stopped
                        symbol = frame.GetSymbol();
                        if symbol:
                            # We do have a symbol, print some info for the symbol
                            print (symbol)
                            # Now get all instructions for this symb        ol and print them
                            insts = symbol.GetInstructions(target)
                            disassemble_instructions (insts)
                    registerList = frame.GetRegisters()
                    print('Frame registers (size of register set = %d):' % registerList.GetSize())
                    for value in registerList:
                        #print value
                        print('%s (number of children = %d):' % (value.GetName(), value.GetNumChildren())        )
                        for child in value:
                            print('Name: ', child.GetName(), ' Value: ', child.GetValue())
                            
            print('Hit the breakpoint at main, enter to continue and wait for program to exit or Ctrl-D quit to terminate the program')
            next = sys.stdin.readline()
            if not next or next.rstrip('') == 'quit':
                print('Terminating the inferior process...')
                process.Kill()
            else:
                # Now continue to the program exit
                process.Continue()
                # When we return from the above function we will hopefully be at the
                # program exit. Print out some process info
                print (process)
        elif state == lldb.eStateExited:
            print('Didnt hit the breakpoint at main, program has exited...')
        else:
            print('Unexpected process state: %s, killing process...' % debugger.StateAsCString (state))
            process.Kill()
#Sometimes you need to create an empty target that will get filled in later.  The most common use for this
#is to attach to a process by name or pid where you don't know the executable up front.  The most convenie        nt way
#to do this is:

# target = debugger.CreateTarget('')
# error = lldb.SBError()
# process = target.AttachToProcessWithName(debugger.GetListener(), 'PROCESS_NAME', False, error)
#    or the equivalent arguments for AttachToProcessWithID.
