WorldEdit can function as a replacement for VisualMunge, allowing you to munge your map (or entire mod) without leaving the editor. Like VisualMunge (or ZeroMunge) it depends on the munge executables from the modtools, unlike VisualMunge however it doesn't depend on the .bat files.

It also has basic multithreading support. For instance two sides can be munged at once and inside those sides both pc_ModelMunge and pc_TextureMunge could
be running at the same time. But there is no deeper parallelism than that (so within a side all textures will still be munged by the same single threaded 
pc_TextureMunge instance).

Even so it is still capable of drastically reducing the time large munges take and can have noticeable but smaller impacts on smaller munges (like say 
munging to update a single world). On my PC (SATA SSD, DDR4 3600MHz, Ryzen 9 5900XT) with the basic multithreading a munge of all the game's stock game files (excluding sounds) takes 
`48.268s`, down from `354.927s`.

# Errors and Warnings
WorldEdit will try parse errors from munge tools into a structured format and display them in the Errors and Warnings table in the Munge Manager. For custom tools it may fail to parse errors, you can view the raw error stream in 
these cases by clicking Display Standard Error Output above the output box.

For structured errors you can go to the problem file in Explorer (if it exists) by clicking on them. Error/warning messages can be copied by right clicking them and pressing Copy.

# Sounds

## Sound Stream Localizations

A feature I would guess few if any mods have used is localizations for sounds. Basically you create extra stream files of the form `{NAME}.stm_{LANGUAGE}`, i.e `cis_unit_vo_quick.stm_french` that references the localized sounds.
 
Then when you call `soundmungedir.bat` with the appropiate environment variables set it'll use the localized sounds instead of the english ones. WorldEdit 
takes a slightly different approach that let's you mirror the way the GoG build of the game ships localized sounds.

Importantly it still follows the same naming `{NAME}.stm_{LANGUAGE}` convention but can munge localized sounds at the same time as others.

At the bottom of the Sound settings in the Munge window you'll find a Localizations section. It won't have anything listed to start with except two input boxes, one for Language and one 
for Output Directory. So if you were wanting to produce a french localization that used `cis_unit_vo_quick.stm_french` you would put `french` into the language box and 
then `fr` (or whatever works for you, this is what GoG uses) into the Output Directory. 

You should then be able to click Add and it should appear in the list of localizations. Now if we pretend we're munging `global.lvl` after we munge we should 
now have both `_LVL_PC\sound\global.lvl` (for the unlocalized, typically english sounds) and `_LVL_PC\sound\fr\global.lvl` for the localized sounds.

You can then pack .lvls however you want for localized versions of your mod or possibly even use `ScriptCB_GetLanguage` in your mission script keep one version of your mod and
just load the right streams as needed.

## PC Sound common.bnk

On PC in BF2 the game's stock sound banks are all packed into one giant one called `common.bnk` and left out of the regular sound.lvls. WorldEdit supports
producing a `common.bnk`. In the Sound settings in the Munge window scroll down to Shared and just below it should be Enable Common Bank. Turn it on and your 
sound banks will be placed into a `common.bnk` just like the stock game.

While this is here for completeness in case anyone ever wanted to reproduce the stock game files in WorldEdit there is likely nothing stopping you from using this in an
addon either. Just load `common.bnk` in your mission script before your regular sound.lvls, using the same calls you would to load them.

# Custom Munge Tools

While WorldEdit completely supports rebuilding the game's stock files sometimes you might want to run extra tools during the munge process. With VisualMunge, ZeroMunge or the raw .bats this is easy, just edit the .bat file and you're good to go.
WorldEdit doesn't use the .bat files however so things naturally work a little different.

Instead you can specify lists of commands to run at various points in the munge process. After munging side files but before packing, after packing children, etc. If these commands produce files outside the normal _BUILD directories you 
can also specify extra directories to be cleaned when you hit clean.

To use these you need to open the Munge Config Editor, the button to open it is below the one to open the Munge Manager.

## Custom Munge Commands

Custom munge commands are just command lines that will be passed to the OS after expanding their environment variables. Before expanding regular environment variables WorldEdit has a small set of context dependent variables it will first expand. They are listed in the table below.

Custom commands are executed in order (top to bottom) and their is no parallelism, each command will only be called after the previous one has completed. Excluding detach commands. 

### Custom Munge Command Variables

| Variable | Expansion |
| -------- | --------- |
| `%TOOLSFL_BIN_PATH%` | Expands to the path to `MODTOOLS\ToolsFL\bin`. |
| `%PROJECT_PATH%` | Expands to the project path, aka the `data_MOD` folder. |
| `%SOURCE_PATH%` | Expands to the source path for what is currently being munged. I.e `data_MOD\Common` when munging common or `data_MOD\sides\REP` when munging the `rep` side. |
| `%OUTPUT_PATH%` | Expands to the output path for what is currently being munged, this is the folder munged files are placed into. Generally it will be under `data_MOD\_BUILD` somewhere |
| `%LVL_OUTPUT_PATH%` | Expands to the output path for .lvls for what is currently being munged. I.e `data_MOD\_LVL_PC\side`.
| `%PLATFORM%` | Expands to the platform being munged, i.e `pc`. |
| `%COMMON_FILES%` | Expands to the list of common file manifests for what is currently being munged. This is normally either empty or `%PROJECT_PATH%\_BUILD\Common\MUNGED\%PLATFORM%\core.files %PROJECT_PATH%\\_BUILD\Common\MUNGED\%PLATFORM%\common.files %PROJECT_PATH%\_BUILD\Common\MUNGED\%PLATFORM%\ingame.files`.

> Example: Assuming modtools at `C:\BF2_ModTools` and a mod folder of `data_MOD` then
> `CustomTool -input %SOURCE_PATH% -output %OUTPUT_PATH%` when munging the `rep` side will expand to `CustomTool C:\BF2_ModTools\data_MOD_\sides\rep C:\BF2_ModTools\data_MOD\_BUILD\Sides\REP\MUNGED\PC`.

### Custom Munge Command Settings

For crossplatform modders you can also set commands to only execute when munging specific platforms. See the `Platform` setting.

If your command is not actually part of the munge process and is just for opening a log or something you should enable the `Detach` setting. This will cause the munge runner to not wait for the launched process to exit before continuing.

## Custom Clean Directories

Custom clean directories are are simpler than commands. You specify a directory, relative or absolute and everything in that directory will be removed.

### Custom Clean Directories Variables

| Variable | Expansion |
| -------- | --------- |
| `%PROJECT_PATH%` | Expands to the project path, aka the `data_MOD` folder. |
| `%BASE_PATH%` | Expands to the path that is currently being cleaned, this is the folder that munged were placed into. Generally it will be under `data_MOD\_BUILD` somewhere |
| `%PLATFORM%` | Expands to the platform being munged, i.e `pc`. |

> Example: `%PROJECT_PATH%..\PC_SP` when cleaning the `rep` side will clean everything in `DATA_MOD\_BUILD\Sides\REP\MUNGED\PC_SP`.