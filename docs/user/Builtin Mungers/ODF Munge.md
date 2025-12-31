# ODF Munge

WorldEdit's builtin ODF should be able to handle all well formed ODFs. It is slightly less forgiving than `OdfMunge` from the modtools but this isn't necessarily a bad thing. It produces clear error messages when it can't parse the ODF and will supply the line number of the error.

In terms of speed in WorldEdit's builtin one and `OdfMunge` the overwhelming majority of time (92%+) is spent doing IO and they come out about even. Except for when doing munges where no or few ODFs actually need munging, in this case WorldEdit's starts to pull ahead slightly.

## Errors

### ODF_LOAD_IO_ERROR

An error occured while opening the .odf file. It may be in use by another app or you may not have permission to open it.

### ODF_LOAD_PARSE_ERROR

Failed to parse the ODF! This is most likely because of a syntax error in the ODF. The Exception Message in the error output will have more information.

### REQ_WRITE_IO_OPEN_ERROR

Failed to open .class.req file for output! Make sure the munge out directory is accessible and that nothing currently has the output .class.req file open.

### REQ_WRITE_IO_GENERIC_ERROR

An unexpected IO error has occured while writing the ODF's .class.req file. The Exception Message in the error output may have more information and context.

### WRITE_IO_OPEN_ERROR

Failed to open .class file for output! Make sure the munge out directory is accessible and that nothing currently has the output .class file open.

### WRITE_IO_GENERIC_ERROR

An unexpected IO error has occured while writing the munged ODF. The Exception Message in the error output may have more information and context.
