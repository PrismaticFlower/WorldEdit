using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Text;

string[] targetShaderModels = { "6_1", "6_6" };

if (args.Length != 5)
{
	Console.WriteLine("ShaderCompiler <Path to .shaders File> <Path to Intermediate Folder> <Path to Output Folder> <Path to Output Shader List .cpp> <Path to dxc.exe>");

	return 1;
}

string fullShadersPath = Path.GetFullPath(args[0]);
string intermediateFolder = args[1];
string outputFolder = args[2];
string shaderListOutputPath = args[3];
string dxcPath = args[4];

string shaderDependenciesPath = $"{intermediateFolder}\\shaders.dep";

var makeOutputFolder = Task.Run(() => Directory.CreateDirectory(outputFolder));
var makeIntermediateFolder = Task.Run(() => Directory.CreateDirectory(intermediateFolder));
var loadShadersDependencies = Task.Run(() =>
{
	Dictionary<string, ShaderDependencies> shadersDependencies = new Dictionary<string, ShaderDependencies>();

	if (!File.Exists(shaderDependenciesPath)) return shadersDependencies;

	string? name = null;
	List<DateTime> fileLastWrites = new List<DateTime>();
	List<string> filePaths = new List<string>();

	foreach (string line in File.ReadAllLines(shaderDependenciesPath))
	{
		if (line.StartsWith("\t"))
		{
			string[] tokens = line.Trim().Split(" ");
			string lastWriteSring = tokens[0];
			string filePath = tokens[1];

			DateTime lastWriteTime = DateTime.FromBinary(long.Parse(lastWriteSring));

			fileLastWrites.Add(lastWriteTime);
			filePaths.Add(filePath);
		}
		else
		{
			if (name != null)
			{
				shadersDependencies[name] = ShaderDependencies.CreateFromLists(filePaths, fileLastWrites);

				fileLastWrites = new List<DateTime>();
				filePaths = new List<string>();
			}

			name = line;
		}
	}

	if (name != null && name.Length > 0)
	{
		shadersDependencies[name] = ShaderDependencies.CreateFromLists(filePaths, fileLastWrites);
	}

	return shadersDependencies;
});

string[] shadersFileLines = File.ReadAllLines(args[0]);

List<ShaderDef> shaderDefs = new List<ShaderDef>(shadersFileLines.Length);

bool compilationError = false;

for (int i = 0; i < shadersFileLines.Length; ++i)
{
	string line = shadersFileLines[i].TrimStart().TrimEnd();

	if (line.StartsWith("//")) continue;
	if (line.Length == 0) continue;

	string[] shaderArgs = line.Split(" ", StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries);

	ShaderDef shader = new ShaderDef
	{
		name = shaderArgs[0]
	};

	if ((shaderArgs.Length - 1) % 3 != 0)
	{
		Console.Error.WriteLine($"{fullShadersPath}({i}): Shader ('{shader.name}') has too few arguments.");

		goto BadShaderDef;
	}

	foreach (string[] shaderArg in shaderArgs.Skip(1).Chunk(3))
	{
		string key = shaderArg[0];
		string split = shaderArg[1];
		string value = shaderArg[2];

		if (split != "=")
		{
			Console.Error.WriteLine($"{fullShadersPath}({i}): Shader ('{shader.name}') has argument ('{key}') missing '=' to set value.");

			goto BadShaderDef;
		}

		if (key == ".entrypoint")
		{
			shader.entryPoint = value;
		}
		else if (key == ".file")
		{
			shader.file = value;
		}
		else
		{
			Console.Error.WriteLine($"{fullShadersPath}({i}): Shader ('{shader.name}') has unknown argument ('{key} = '{value}').");

			goto BadShaderDef;
		}
	}

	shader.entryPoint ??= "main";
	shader.file ??= $"{shader.name}.hlsl";

	if (shader.name.EndsWith("VS"))
	{
		shader.targetPrefix = "vs_";
	}
	else if (shader.name.EndsWith("HS"))
	{
		shader.targetPrefix = "hs_";
	}
	else if (shader.name.EndsWith("DS"))
	{
		shader.targetPrefix = "ds_";
	}
	else if (shader.name.EndsWith("GS"))
	{
		shader.targetPrefix = "gs_";
	}
	else if (shader.name.EndsWith("PS"))
	{
		shader.targetPrefix = "ps_";
	}
	else if (shader.name.EndsWith("CS"))
	{
		shader.targetPrefix = "cs_";
	}
	else if (shader.name.EndsWith("LIB"))
	{
		shader.targetPrefix = "lib_";
	}
	else
	{
		Console.Error.WriteLine($"{fullShadersPath}({i}): Shader ('{shader.name}') has invalid target suffix.");

		goto BadShaderDef;
	}

	shaderDefs.Add(shader);

	continue;

BadShaderDef:
	compilationError = true;
}

makeOutputFolder.Wait();
makeIntermediateFolder.Wait();

string? shadersFolder = Path.GetDirectoryName(fullShadersPath);

if (shadersFolder == null)
{
	Console.Error.WriteLine("Can't get shaders directory.");

	return 1;
}

string[] literalFormatTable = new string[256];

for (uint i = 0; i < 256; ++i)
{
	literalFormatTable[i] = $"{i:x}";
}

Dictionary<string, ShaderDependencies> cachedShadersDependencies = loadShadersDependencies.Result;
Dictionary<string, ShaderDependencies> shadersDependencies = new Dictionary<string, ShaderDependencies>(shaderDefs.Count);

Parallel.For(0, shaderDefs.Count, async i =>
{
	ShaderDef shader = shaderDefs[i];
	ShaderDependencies? dependencies = cachedShadersDependencies.GetValueOrDefault(shader.name);

	if (dependencies != null && !dependencies.OutOfDate())
	{
		lock (shadersDependencies)
		{
			shadersDependencies[shader.name] = dependencies;
		}

		return;
	}

	foreach (string targetShaderModel in targetShaderModels)
	{
		Console.WriteLine($"Compiling {shader.name}({shader.file}:{shader.entryPoint}) => {shader.targetPrefix}{targetShaderModel}");

		StringBuilder dxcArgsBase = new StringBuilder();

		dxcArgsBase.Append(shadersFolder);
		dxcArgsBase.Append("\\");
		dxcArgsBase.Append(shader.file);
		dxcArgsBase.Append(" ");

		dxcArgsBase.Append("-E ");
		dxcArgsBase.Append(shader.entryPoint);
		dxcArgsBase.Append(" ");

		dxcArgsBase.Append("-T ");
		dxcArgsBase.Append(shader.targetPrefix);
		dxcArgsBase.Append(targetShaderModel);
		dxcArgsBase.Append(" ");

		StringBuilder dxcArgs = new StringBuilder();

		dxcArgs.Append(dxcArgsBase);

		dxcArgs.Append("-Fo ");
		dxcArgs.Append(intermediateFolder);
		dxcArgs.Append("\\");
		dxcArgs.Append(shader.name);
		dxcArgs.Append("_");
		dxcArgs.Append(targetShaderModel);
		dxcArgs.Append(".dxil ");

		dxcArgs.Append("-Zs -Fd ");
		dxcArgs.Append(intermediateFolder);
		dxcArgs.Append("\\");
		dxcArgs.Append(shader.name);
		dxcArgs.Append("_");
		dxcArgs.Append(targetShaderModel);
		dxcArgs.Append(".pdb ");

		dxcArgs.Append("-Qstrip_reflect");


		ProcessStartInfo startInfo = new ProcessStartInfo(dxcPath, dxcArgs.ToString());

		startInfo.RedirectStandardError = true;
		startInfo.UseShellExecute = false;

		Process? process = Process.Start(startInfo);

		if (process == null)
		{
			Console.Error.WriteLine($"Failed to launch DXC for '{shader.name}'.");

			compilationError = true;

			return;
		}

		string errorMessages = process.StandardError.ReadToEnd();

		process.WaitForExit();

		if (process.ExitCode != 0)
		{
			if (errorMessages.Length != 0)
			{
				Console.Error.WriteLine(errorMessages);
			}

			compilationError = true;

			return;
		}
	}

	Task<byte[]>[] dxilBytesRead = { File.ReadAllBytesAsync($"{intermediateFolder}\\{shader.name}_{targetShaderModels[0]}.dxil"), File.ReadAllBytesAsync($"{intermediateFolder}\\{shader.name}_{targetShaderModels[1]}.dxil") };

	Task.WaitAll(dxilBytesRead);

	byte[][] dxilBytes = { dxilBytesRead[0].Result, dxilBytesRead[1].Result };

	StringBuilder cppBuilder = new StringBuilder();

	cppBuilder.Append("#include \"shader_def.hpp\"\n");
	cppBuilder.Append("\n");
	cppBuilder.Append("namespace we::graphics::shaders {\n");
	cppBuilder.Append("\n");
	cppBuilder.Append($"extern const char {shader.name}_{targetShaderModels[0]}_dxil_bytes[{dxilBytes[0].Length + 1}];\n");
	cppBuilder.Append($"extern const char {shader.name}_{targetShaderModels[1]}_dxil_bytes[{dxilBytes[1].Length + 1}];\n");
	cppBuilder.Append("\n");
	cppBuilder.Append($"auto {shader.name}() noexcept -> shader_def\n{{\n");
	cppBuilder.Append($"   return {{\n");
	cppBuilder.Append($"      .name = \"{shader.name}\",\n");
	cppBuilder.Append($"      .entrypoint = L\"{shader.entryPoint}\",\n");
	cppBuilder.Append($"      .target_{targetShaderModels[0]} = L\"{shader.targetPrefix}{targetShaderModels[0]}\",\n");
	cppBuilder.Append($"      .target_{targetShaderModels[1]} = L\"{shader.targetPrefix}{targetShaderModels[1]}\",\n");
	cppBuilder.Append($"      .file = L\"{shader.file}\",\n");
	cppBuilder.Append($"      .dxil_{targetShaderModels[0]} = {{reinterpret_cast<const std::byte*>({shader.name}_{targetShaderModels[0]}_dxil_bytes),\n");
	cppBuilder.Append($"                      sizeof({shader.name}_{targetShaderModels[0]}_dxil_bytes) - 1}},\n");
	cppBuilder.Append($"      .dxil_{targetShaderModels[1]} = {{reinterpret_cast<const std::byte*>({shader.name}_{targetShaderModels[1]}_dxil_bytes),\n");
	cppBuilder.Append($"                      sizeof({shader.name}_{targetShaderModels[1]}_dxil_bytes) - 1}},\n");
	cppBuilder.Append("   };\n}\n\n");

	string[] cppDxilStart = {
		$"const char {shader.name}_{targetShaderModels[0]}_dxil_bytes[{dxilBytes[0].Length + 1}] = \"",
		$"const char {shader.name}_{targetShaderModels[1]}_dxil_bytes[{dxilBytes[1].Length + 1}] = \"",
	};

	string cppDxilTail = "\";\n\n";

	byte cppClose = (byte)'}';

	List<byte> cppBytes = new List<byte>(cppBuilder.Length + cppDxilStart[0].Length + cppDxilStart[1].Length + (dxilBytes.Length * 4) + (cppDxilTail.Length * 2) + 1);

	cppBytes.AddRange(cppBuilder.ToString().Select(ch => (byte)ch));

	for (int j = 0; j < 2; ++j)
	{
		cppBytes.AddRange(cppDxilStart[j].Select(ch => (byte)ch));

		foreach (byte value in dxilBytes[j])
		{
			cppBytes.Add((byte)'\\');
			cppBytes.Add((byte)'x');
			cppBytes.AddRange(literalFormatTable[value].Select(ch => (byte)ch));
		}

		cppBytes.AddRange(cppDxilTail.Select(ch => (byte)ch));
	}

	cppBytes.Add(cppClose);

	using (FileStream file = new FileStream($"{outputFolder}\\{shader.name}.cpp", FileMode.Create))
	{
		file.Write(CollectionsMarshal.AsSpan(cppBytes));
	}

	File.SetLastAccessTimeUtc($"{outputFolder}\\{shader.name}.cpp", DateTime.UtcNow);

	StringBuilder dxcMDArgs = new StringBuilder();

	dxcMDArgs.Append(shadersFolder);
	dxcMDArgs.Append("\\");
	dxcMDArgs.Append(shader.file);
	dxcMDArgs.Append(" ");

	dxcMDArgs.Append("-E ");
	dxcMDArgs.Append(shader.entryPoint);
	dxcMDArgs.Append(" ");

	dxcMDArgs.Append("-T ");
	dxcMDArgs.Append(shader.targetPrefix);
	dxcMDArgs.Append(targetShaderModels[1]);
	dxcMDArgs.Append(" ");

	dxcMDArgs.Append("-M");

	ProcessStartInfo startInfoMD = new ProcessStartInfo(dxcPath, dxcMDArgs.ToString());

	startInfoMD.RedirectStandardOutput = true;
	startInfoMD.UseShellExecute = false;

	Process? processMD = Process.Start(startInfoMD);

	if (processMD == null)
	{
		Console.Error.WriteLine($"Failed to launch DXC for '{shader.name}'.");

		compilationError = true;

		return;
	}

	lock (shadersDependencies)
	{
		shadersDependencies[shader.name] = ShaderDependencies.CreateFromDXCOutput(processMD.StandardOutput.ReadToEnd());
	}

	processMD.WaitForExit();

	if (processMD.ExitCode != 0)
	{
		compilationError = true;

		return;
	}
});

if (!shaderDefs.All(shader => cachedShadersDependencies.ContainsKey(shader.name)))
{
	using (StreamWriter file = new StreamWriter(shaderListOutputPath))
	{
		file.Write("#include \"shader_list.hpp\"\n");
		file.Write("\n");
		file.Write("namespace we::graphics {\n");
		file.Write("\n");
		file.Write("namespace shaders {\n");
		file.Write("\n");

		foreach (ShaderDef shader in shaderDefs)
		{
			file.Write($"auto {shader.name}() noexcept -> shader_def;\n");
		}

		file.Write("\n");
		file.Write("}\n");
		file.Write("\n");

		file.Write("std::initializer_list<shader_def> shader_list = {\n");

		foreach (ShaderDef shader in shaderDefs)
		{
			file.Write($"shaders::{shader.name}(),\n");
		}

		file.Write("};\n");
		file.Write("\n");

		file.Write("}");
	}

}

using (StreamWriter file = new StreamWriter(shaderDependenciesPath))
{
	foreach (var (shader, shaderDependencies) in shadersDependencies)
	{
		file.WriteLine(shader);

		foreach (var (filePath, lastWriteTime) in shaderDependencies.Files.Zip(shaderDependencies.FilesLastWriteTime))
		{
			file.WriteLine($"\t{lastWriteTime.ToBinary()} {filePath}");
		}
	}
}

return compilationError ? 1 : 0;

struct ShaderDef
{
	public string name;
	public string entryPoint;
	public string file;
	public string targetPrefix;
};

class ShaderDependencies
{
	public static ShaderDependencies CreateFromLists(List<string> files, List<DateTime> filesLastWriteTime)
	{
		Debug.Assert(files.Count == filesLastWriteTime.Count);

		return new ShaderDependencies
		{
			files = files,
			filesLastWriteTime = filesLastWriteTime
		};
	}

	public static ShaderDependencies CreateFromDXCOutput(string output)
	{
		string[] parts = output.Split(" ", StringSplitOptions.TrimEntries);

		ShaderDependencies shaderDependencies = new ShaderDependencies
		{
			files = new List<string>(parts.Length),
			filesLastWriteTime = new List<DateTime>(parts.Length)
		};

		foreach (string part in parts.Skip(1))
		{
			if (part == "\\") continue;
			if (!File.Exists(part)) continue;

			shaderDependencies.files.Add(part);
			shaderDependencies.filesLastWriteTime.Add(File.GetLastWriteTimeUtc(part));
		}


		return shaderDependencies;
	}

	public bool OutOfDate()
	{
		foreach (var (file, lastWrite) in files.Zip(filesLastWriteTime))
		{
			try
			{
				if (File.GetLastWriteTimeUtc(file) != lastWrite) return true;
			}
			catch (Exception)
			{
				return true;
			}
		}

		return false;
	}

	public List<string> Files { get { return files; } }
	public List<DateTime> FilesLastWriteTime { get { return filesLastWriteTime; } }

	List<string> files = new List<string>();
	List<DateTime> filesLastWriteTime = new List<DateTime>();
}
