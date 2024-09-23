// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

using System.IO.Abstractions;
using Azure.DataApiBuilder.Config;
using Cli.Commands;
using CommandLine;
using Microsoft.Extensions.Logging;

namespace Cli
{
    /// <summary>
    /// Main class for CLI
    /// </summary>
    public class Program 
    {
        public const string PRODUCT_NAME = "Microsoft.DataApiBuilder";

        /// <summary>
        /// Main CLI entry point
        /// </summary>
        /// <param name="args">CLI arguments</param>
        /// <returns>0 on success, -1 on failure.</returns>
        public static int Main(string[] args)
        {
            // Load environment variables from .env file if present.
            DotNetEnv.Env.Load();

            // Setting up Logger for CLI.
            ILoggerFactory loggerFactory = new LoggerFactory();
            loggerFactory.AddProvider(new CustomLoggerProvider());

            ILogger<Program> cliLogger = loggerFactory.CreateLogger<Program>();
            ILogger<ConfigGenerator> configGeneratorLogger = loggerFactory.CreateLogger<ConfigGenerator>();
            ILogger<Utils> cliUtilsLogger = loggerFactory.CreateLogger<Utils>();
            ConfigGenerator.SetLoggerForCliConfigGenerator(configGeneratorLogger);
            Utils.SetCliUtilsLogger(cliUtilsLogger);
            IFileSystem fileSystem = new FileSystem();
            FileSystemRuntimeConfigLoader loader = new(fileSystem);
#if false
init --database-type "mssql" 
--connection-string "Server=rcvw-db-server.database.windows.net;Database=rcvw-db;User ID=dba;Password=B@ttelle;TrustServerCertificate=true"
--host-mode "Development"
#endif
#if false
dab add HriStatus --source dbo.hri_crossings --permissions "anonymous:*"
#endif
            int rv = Execute(args, cliLogger, fileSystem, loader);
            return rv;
        }

        public static int Execute(string[] args, ILogger cliLogger, IFileSystem fileSystem, FileSystemRuntimeConfigLoader loader)
        {
            // To know if `--help` or `--version` was requested.
            bool isHelpOrVersionRequested = false;

            Parser parser = new(settings =>
            {
                settings.CaseInsensitiveEnumValues = true;
                settings.HelpWriter = Console.Out;
            });

            // Parsing user arguments and executing required methods.
            ParserResult<object>? result = parser.ParseArguments<InitOptions, AddOptions, UpdateOptions, StartOptions, ExportOptions>(args)
                .WithParsed((Action<InitOptions>)(options => options.Handler(cliLogger, loader, fileSystem)))
                .WithParsed((Action<AddOptions>)(options => options.Handler(cliLogger, loader, fileSystem)))
                .WithParsed((Action<UpdateOptions>)(options => options.Handler(cliLogger, loader, fileSystem)))
                .WithParsed((Action<StartOptions>)(options => options.Handler(cliLogger, loader, fileSystem)))
                .WithParsed((Action<ExportOptions>)(options => Exporter.Export(options, cliLogger, loader, fileSystem)))
                .WithNotParsed(err =>
                {
                    /// System.CommandLine considers --help and --version as NonParsed Errors
                    /// ref: https://github.com/commandlineparser/commandline/issues/630
                    /// This is a workaround to make sure our app exits with exit code 0,
                    /// when user does --help or --versions.
                    /// dab --help -> ErrorType.HelpVerbRequestedError
                    /// dab [command-name] --help -> ErrorType.HelpRequestedError
                    /// dab --version -> ErrorType.VersionRequestedError
                    List<Error> errors = err.ToList();
                    if (errors.Any(e => e.Tag == ErrorType.VersionRequestedError
                                        || e.Tag == ErrorType.HelpRequestedError
                                        || e.Tag == ErrorType.HelpVerbRequestedError))
                    {
                        isHelpOrVersionRequested = true;
                    }
                });

            return ((result is Parsed<object>) || (isHelpOrVersionRequested)) ? 0 : -1;
        }
    }
}
