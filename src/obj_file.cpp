#include "Logger.h"
#include "mesh.h"
#include "colors.h"

static int objFileLoaderParser(mesh_t* mesh, const char* content);
static void objFileLoaderLineParser(mesh_t* mesh, char* line, uint32_t currentLine);
static int objFileLoaderExecuteLine(mesh_t* mesh, int argc, char* argv[], uint32_t currentLine);

static uint32_t objFileLoaderIgnoredLines = 0;
static uint32_t objFileLoaderLoadedVertices = 0;
static uint32_t objFileLoaderLoadedVerticesNormal = 0;
static uint32_t objFileLoaderLoadedVerticesTexture = 0;
static uint32_t objFileLoaderLoadedFaces = 0;
static uint32_t objFileLoaderTotalTri = 0;


void loadOBJFile(mesh_t* mesh, const char* filepath){
    FILE* fp;
    size_t fileSize;
    char* fileBuff;
    fp = fopen(filepath, "rt");
    if (fp)
    {
        fseek(fp, 0, SEEK_END);
        fileSize = ftell(fp);
        /* Add one byte to the size to make sure it is null terminated */
        fileBuff = (char*)calloc(fileSize + 1, 1);
        fseek(fp, 0, SEEK_SET);
        fileSize = fread(fileBuff, 1, fileSize, fp);
        fclose(fp);

        objFileLoaderIgnoredLines = 0;
        objFileLoaderLoadedVertices = 0;
        objFileLoaderLoadedVerticesNormal = 0;
        objFileLoaderLoadedVerticesTexture = 0;
        objFileLoaderLoadedFaces = 0;
        objFileLoaderTotalTri = 0;


        if (objFileLoaderParser(mesh, fileBuff)){
            //Log(TLOG_ERROR, "OBJLoader", "Errors occurred while opening the file '%s'.", filepath);
        }

        //Log(TLOG_VERBOSE, "OBJLoader", "OBJ File '%s' loaded. Some stats:", filepath);
        //Log(TLOG_VERBOSE, "OBJLoader", "Total ignored lines         : %lu", objFileLoaderIgnoredLines);
        //Log(TLOG_VERBOSE, "OBJLoader", "Number of vertices          : %lu", objFileLoaderLoadedVertices);
        //Log(TLOG_VERBOSE, "OBJLoader", "Number of normal vertices   : %lu", objFileLoaderLoadedVerticesNormal);
        //Log(TLOG_VERBOSE, "OBJLoader", "Number of texture vertices  : %lu", objFileLoaderLoadedVerticesTexture);
        //Log(TLOG_VERBOSE, "OBJLoader", "Number of faces in file     : %lu", objFileLoaderLoadedFaces);
        //Log(TLOG_VERBOSE, "OBJLoader", "Number of created triangles : %lu", objFileLoaderTotalTri);

        free(fileBuff);

        updateFaceUVValues(mesh);
    }
    else
    {
        //Logger:Err(fprintf("OBJLoader", "Can't open/find the file '%s'.", filepath));
    }
}

#define MAX_LINE_LENGTH 512
static int objFileLoaderParser(mesh_t* mesh, const char* content)
{
    /* I don't think we will handle lines of more than 512 characters... */
    char lineBuff[MAX_LINE_LENGTH];
    uint32_t currentLineNum = 1;
    uint32_t totalLength = strlen(content);
    /* Need to process line by line */
    const char* bufferPos = content;
    const char* lineNewline;
    while (*bufferPos != '\0')
    {
        uint32_t lineLength;
        lineNewline = strchr(bufferPos, '\n');
        if (lineNewline == NULL)
        {
            /* We are on the last line */
            lineLength = strlen(bufferPos);
        }
        else
        {
            lineLength = (lineNewline - bufferPos);
        }
        if (lineLength >= MAX_LINE_LENGTH)
        {
            //Log(TLOG_ERROR, "OBJLoader", "Line %d is too long! (%d)", currentLineNum, lineLength);
            return -1;
        }
        memset(lineBuff, 0, MAX_LINE_LENGTH);
        strncpy(lineBuff, bufferPos, lineLength);

        objFileLoaderLineParser(mesh, lineBuff, currentLineNum);

        bufferPos += lineLength + 1;

        if ((bufferPos - content) >= totalLength)
        {
            /* We are past the length of the buffer, don't need to continue */
            break;
        }
        currentLineNum++;
    }
    return 0;
}

#define MAX_ARGS (15)
/* Parse the line into a couple ofr argc/argv using space as argument separator */
static void objFileLoaderLineParser(mesh_t* mesh, char* line, uint32_t currentLine)
{
    char* argv[MAX_ARGS];
    uint32_t argc = 0;

    char* buffer = line;
    uint32_t lineLength = strlen(line);
    uint32_t linePos = 0;

    /* First argument */
    argv[argc++] = line;

    while (linePos < lineLength)
    {
        char* next = strchr(buffer, ' ');
        if (next != NULL)
        {
            *next = '\0';
            linePos = next - line;
            buffer = next + 1;
            /* Skip empty strings as it mean multiple spaces */
            if (strlen(buffer) > 0)
            {
                argv[argc++] = buffer;
            }
        }
        else
        {
            linePos = lineLength;
        }
    }

    if (objFileLoaderExecuteLine(mesh, argc, argv, currentLine))
    {
        objFileLoaderIgnoredLines++;
    }
}

static int objFileLoaderParseFaceVertex(mesh_t* mesh, char* buf, uint32_t* v, uint32_t* vt, uint32_t* vn)
{
    uint32_t bufPos = 0;
    uint32_t lineLength = strlen(buf);
    *vt = INT32_MAX;
    *vn = INT32_MAX;
    int ret = 0;
    int token = 0;

    while (bufPos < lineLength)
    {
        char* next = strchr(buf, '/');
        if (next != NULL)
        {
            *next = '\0';
            bufPos = next - buf;
        }
        else
        {
            bufPos = lineLength;
        }

        if (strlen(buf) > 0)
        {
            switch (token)
            {
            case 0: *v = atol(buf); break;
            case 1: *vt = atol(buf); break;
            case 2: *vn = atol(buf); break;
            default:
                //Log(TLOG_ERROR, "OBJLoader", "Too many entry for a face vertex!");
                ret = 1;
                break;
            }
        }
        buf = next + 1;
        token++;
    }

    return ret;
}

/* Actually execute the line */
int objFileLoaderExecuteLine(mesh_t* mesh, int argc, char* argv[], uint32_t currentLine)
{
    int ret = 1;
    if (strncmp(argv[0], "v", 2) == 0)
    {
        /* Vertice entry */
        if (argc != 4)
        {
            //Log(TLOG_ERROR, "OBJLoader", "Malformed file at line %d: Vertices expect 3 parameters!", currentLine);
        }
        else
        {
            vec3_t vertex = { atof(argv[1]), atof(argv[2]), atof(argv[3]) };
            mesh->vertices.push_back(vertex);
            objFileLoaderLoadedVertices++;
            ret = 0;
        }
    }
    /*else if (strncmp(argv[0], "vn", 3) == 0)
    {
        // Vertice Normal entry 
        if (argc != 4)
        {
            //Log(TLOG_ERROR, "OBJLoader", "Malformed file at line %d: Vertices normal expect 3 parameters!", currentLine);
        }
        else
        {
            //vec3_t vertex = { atof(argv[1]), atof(argv[2]), atof(argv[3]) };
           // mesh->normalVertices.push_back(vertex);
            //objFileLoaderLoadedVerticesNormal++;
           // ret = 0;
        }
    }*/

    /*else if (strncmp(argv[0], "vt", 3) == 0)
    {
        // Vertice Normal entry 
        if ((argc < 3) || (argc > 4))
        {
            //Log(TLOG_ERROR, "OBJLoader", "Malformed file at line %d: Vertices texture expect 2 or 3 parameters!", currentLine);
        }
        else
        {
            tex2_t vertex = { atof(argv[1]), atof(argv[2]) };
            mesh->textureVertices.push_back(vertex);
            objFileLoaderLoadedVerticesTexture++;
            ret = 0;
        }
    }*/
    else if (strncmp(argv[0], "f", 2) == 0)
    {
        /* Faces entry */
        int i;
        uint32_t v[MAX_ARGS], vt[MAX_ARGS], vn[MAX_ARGS];
        for (i = 1; i < argc; i++)
        {
            objFileLoaderParseFaceVertex(mesh, argv[i], &v[i], &vt[i], &vn[i]);
        }

        if (argc == 4)
        {
            objFileLoaderTotalTri++;
            objFileLoaderLoadedFaces++;

            face_t face =
            {
                v[1] - 1,
                v[2] - 1,
                v[3] - 1,
                vt[1] - 1 , vt[2] - 1, vt[3] - 1,
                WHITE
            };
            mesh->faces.push_back(face);
            ret = 0;
        }
        else if (argc > 4)
        {
            objFileLoaderLoadedFaces++;
            /* This is not a triangle, so we need to fan it */
            for (i = 2; i < (argc - 1); i++)
            {
                face_t face = {
                    v[1] - 1,
                    v[i] - 1,
                    v[i + 1] - 1,
                    vt[1] - 1, vt[i] - 1, vt[i + 1] - 1,
                    GREEN};
                mesh->faces.push_back(face);

                objFileLoaderTotalTri++;
            }
            ret = 0;
        }
        else
        {
            //Log(TLOG_ERROR, "OBJLoader", "Malformed file at line %d: Too few/many parameters!", currentLine);
        }
    }
    /* We ignore groups for now, may use it later. */
#if 0
    else if (strncmp(argv[0], "g", 2) == 0)
    {
        if (argc == 2)
        {
            this->addGroup(new Group(argv[1]));
        }
        else
        {
            Log(TLOG_ERROR, "OBJLoader", "Malformed file at line %d: Too few/many parameters!", currentLine);
        }
    }
#endif
    return ret;
}
static void updateFaceUVValues(mesh_t* mesh)
{
    uint32_t i;
    uint32_t faceCount = mesh->faces.size();

    /*if (mesh->textureVertices.size() > 0)
    {
        for (i = 0; i < faceCount; i++)
        {
            face_t* face = &mesh->faces[i];

            face->a_uv.u = mesh->faces[face->a_uv].u;
            face->a_uv.v = mesh->faces[face->a_uv].v;
            face->b_uv.u = mesh->faces[face->b_uv].u;
            face->b_uv.v = mesh->faces[face->b_uv].v;
            face->c_uv.u = mesh->faces[face->c_uv].u;
            face->c_uv.v = mesh->faces[face->c_uv].v;
        }
    }*/
}

