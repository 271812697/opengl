// Copyright (c) 2008-2023 the Urho3D project
// License: MIT

#include <Urho3D/AngelScript/Script.h>
#include <Urho3D/AngelScript/ScriptFile.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Core/ProcessUtils.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/IO/File.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Resource/ResourceCache.h>

#ifdef URHO3D_LUA
#include <Urho3D/LuaScript/LuaScript.h>
#endif

#ifdef WIN32
#include <Urho3D/Engine/WinWrapped.h>
#endif

#include <Urho3D/DebugNew.h>

using namespace Urho3D;

void CompileScript(Context* context, const String& fileName, bool stripDebugSymbols);

int main(int argc, char** argv)
{
    #ifdef WIN32
    const Vector<String>& arguments = ParseArguments(GetCommandLineW());
    #else
    const Vector<String>& arguments = ParseArguments(argc, argv);
    #endif

    bool dumpApiMode = false;
    String sourceTree;
    String outputFile;

    if (arguments.Size() < 1)
        ErrorExit("Usage: ScriptCompiler <input file> [resource path for includes] [-nostrip]\n"
                  "       ScriptCompiler -dumpapi <source tree> <Doxygen output file> [C header output file]");
    else
    {
        if (arguments[0] != "-dumpapi")
            outputFile = arguments[0];
        else
        {
            dumpApiMode = true;
            if (arguments.Size() > 2)
            {
                sourceTree = arguments[1];
                outputFile = arguments[2];
            }
            else
                ErrorExit("Usage: ScriptCompiler -dumpapi <source tree> <Doxygen output file> [C header output file]");
        }
    }

    SharedPtr<Context> context(new Context());
    SharedPtr<Engine> engine(new Engine(context));
    context->RegisterSubsystem(new Script(context));

    // In API dumping mode initialize the engine and instantiate LuaScript system if available so that we
    // can dump attributes from as many classes as possible
    if (dumpApiMode)
    {
        VariantMap engineParameters;
        engineParameters[EP_HEADLESS] = true;
        engineParameters[EP_WORKER_THREADS] = false;
        engineParameters[EP_LOG_NAME] = String::EMPTY;
        engineParameters[EP_RESOURCE_PATHS] = String::EMPTY;
        engineParameters[EP_AUTOLOAD_PATHS] = String::EMPTY;
        engine->Initialize(engineParameters);
    #ifdef URHO3D_LUA
        context->RegisterSubsystem(new LuaScript(context));
    #endif
    }

    auto* log = context->GetSubsystem<Log>();
    // Register Log subsystem manually if compiled without logging support
    if (!log)
    {
        context->RegisterSubsystem(new Log(context));
        log = context->GetSubsystem<Log>();
    }

    log->SetLevel(LOG_WARNING);
    log->SetTimeStamp(false);

    if (!dumpApiMode)
    {
        String path, file, extension;
        SplitPath(outputFile, path, file, extension);

        auto* cache = context->GetSubsystem<ResourceCache>();

        bool stripDebugSymbols = true;
        for (int idx = arguments.Size() - 1; idx > 0; idx--)
        {
            if (arguments[idx] == "-nostrip")
            {
                stripDebugSymbols = false;
                break;
            }
        }

        // Add resource path to be able to resolve includes
        if (arguments.Size() > 1 && arguments[1] != "-nostrip")
            cache->AddResourceDir(arguments[1]);
        else
            cache->AddResourceDir(cache->GetPreferredResourceDir(path));

        if (!file.StartsWith("*"))
            CompileScript(context, outputFile, stripDebugSymbols);
        else
        {
            Vector<String> scriptFiles;
            context->GetSubsystem<FileSystem>()->ScanDir(scriptFiles, path, file + extension, SCAN_FILES, false);
            for (unsigned i = 0; i < scriptFiles.Size(); ++i)
                CompileScript(context, path + scriptFiles[i], stripDebugSymbols);
        }
    }
    else
    {
        if (!outputFile.Empty())
        {
            log->SetQuiet(true);
            log->Open(outputFile);
        }
        // If without output file, dump to stdout instead
        context->GetSubsystem<Script>()->DumpAPI(DOXYGEN, sourceTree);

        // Only dump API as C Header when an output file name is explicitly given
        if (arguments.Size() > 3)
        {
            outputFile = arguments[3];
            log->Open(outputFile);
            context->GetSubsystem<Script>()->DumpAPI(C_HEADER, sourceTree);
        }
    }

    return EXIT_SUCCESS;
}

void CompileScript(Context* context, const String& fileName, bool stripDebugSymbols)
{
    PrintLine("Compiling script file " + fileName);

    File inFile(context, fileName, FILE_READ);
    if (!inFile.IsOpen())
        ErrorExit("Failed to open script file " + fileName);

    ScriptFile script(context);
    script.SetOnlyCompile();
    if (!script.Load(inFile))
        ErrorExit();

    String outFileName = ReplaceExtension(fileName, ".asc");
    File outFile(context, outFileName, FILE_WRITE);
    if (!outFile.IsOpen())
        ErrorExit("Failed to open output file " + fileName);

    script.SaveByteCode(outFile, stripDebugSymbols);
}