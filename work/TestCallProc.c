//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// TestCallProc.c
//   Tests simple call of stored procedure with in, in/out and out variables.
//-----------------------------------------------------------------------------

#include <dpi.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define SQL_TEXT        "begin proc_Test(:1, :2, :3); end;"

static dpiContext *gContext = NULL;
static dpiSampleParams gParams;

//-----------------------------------------------------------------------------
// dpiSamples__fatalError() [INTERNAL]
//   Called when a fatal error is encountered from which recovery is not
// possible. This simply prints a message to stderr and exits the program with
// a non-zero exit code to indicate an error.
//-----------------------------------------------------------------------------
static void dpiSamples__fatalError(const char *message)
{
    fprintf(stderr, "FATAL: %s\n", message);
    exit(1);
}


//-----------------------------------------------------------------------------
// dpiSamples__finalize() [INTERNAL]
//   Destroy context upon process exit.
//-----------------------------------------------------------------------------
static void dpiSamples__finalize(void)
{
    dpiContext_destroy(gContext);
}


//-----------------------------------------------------------------------------
// dpiSamples__getEnvValue()
//   Get parameter value from the environment or use supplied default value if
// the value is not set in the environment. Memory is allocated to accommodate
// the value.
//-----------------------------------------------------------------------------
static void dpiSamples__getEnvValue(const char *envName,
        const char *defaultValue, const char **value, uint32_t *valueLength,
        int convertToUpper)
{
    const char *source;
    uint32_t i;
    char *ptr;

    source = getenv(envName);
    if (!source)
        source = defaultValue;
    *valueLength = strlen(source);
    *value = malloc(*valueLength);
    if (!*value)
        dpiSamples__fatalError("Out of memory!");
    strncpy((char*) *value, source, *valueLength);
    if (convertToUpper) {
        ptr = (char*) *value;
        for (i = 0; i < *valueLength; i++)
            ptr[i] = toupper(ptr[i]);
    }
}


//-----------------------------------------------------------------------------
// dpiSamples_getConn()
//   Connect to the database using the supplied parameters. The DPI library
// will also be initialized, if needed.
//-----------------------------------------------------------------------------
dpiConn *dpiSamples_getConn(int withPool, dpiCommonCreateParams *commonParams)
{
    dpiConn *conn;
    dpiPool *pool;

    // perform initialization
    dpiSamples_getParams();

    // create a pool and acquire a connection
    if (withPool) {
        if (dpiPool_create(gContext, gParams.mainUserName,
                gParams.mainUserNameLength, gParams.mainPassword,
                gParams.mainPasswordLength, gParams.connectString,
                gParams.connectStringLength, commonParams, NULL, &pool) < 0) {
            dpiSamples_showError();
            dpiSamples__fatalError("Unable to create pool.");
        }
        if (dpiPool_acquireConnection(pool, NULL, 0, NULL, 0, NULL,
                    &conn) < 0) {
            dpiSamples_showError();
            dpiSamples__fatalError("Unable to acquire connection from pool.");
        }
        dpiPool_release(pool);

    // or create a standalone connection
    } else if (dpiConn_create(gContext, gParams.mainUserName,
            gParams.mainUserNameLength, gParams.mainPassword,
            gParams.mainPasswordLength, gParams.connectString,
            gParams.connectStringLength, commonParams, NULL, &conn) < 0) {
        dpiSamples_showError();
        dpiSamples__fatalError("Unable to create connection.");
    }

    return conn;
}


//-----------------------------------------------------------------------------
// dpiSamples_getParams()
//   Get parameters set in the environment. The DPI library will also be
// initialized if needed.
//-----------------------------------------------------------------------------
dpiSampleParams *dpiSamples_getParams(void)
{
    dpiErrorInfo errorInfo;

    // DPI_MAJOR_VERSION, DPI_MINOR_VERSION is defined in dpi.h

    if (!gContext) {
        if (dpiContext_create(DPI_MAJOR_VERSION, DPI_MINOR_VERSION, &gContext,
                &errorInfo) < 0) {
            fprintf(stderr, "ERROR: %.*s (%s : %s)\n", errorInfo.messageLength,
                    errorInfo.message, errorInfo.fnName, errorInfo.action);
            dpiSamples__fatalError("Cannot create DPI context.");
        }
        atexit(dpiSamples__finalize);
    }

    dpiSamples__getEnvValue("ODPIC_SAMPLES_MAIN_USER", "heejong",
            &gParams.mainUserName, &gParams.mainUserNameLength, 1);
    dpiSamples__getEnvValue("ODPIC_SAMPLES_MAIN_PASSWORD", "Welcome1",
            &gParams.mainPassword, &gParams.mainPasswordLength, 0);
    dpiSamples__getEnvValue("ODPIC_SAMPLES_PROXY_USER", "odpicdemo_proxy",
            &gParams.proxyUserName, &gParams.proxyUserNameLength, 1);
    dpiSamples__getEnvValue("ODPIC_SAMPLES_PROXY_PASSWORD", "welcome",
            &gParams.proxyPassword, &gParams.proxyPasswordLength, 0);
    dpiSamples__getEnvValue("ODPIC_SAMPLES_CONNECT_STRING", "PDB1",
            &gParams.connectString, &gParams.connectStringLength, 0);
    dpiSamples__getEnvValue("ODPIC_SAMPLES_DIR_NAME", "odpicdemo_dir",
            &gParams.dirName, &gParams.dirNameLength, 1);
    gParams.context = gContext;

    return &gParams;
}


//-----------------------------------------------------------------------------
// dpiSamples_showError()
//   Display the error to stderr.
//-----------------------------------------------------------------------------
int dpiSamples_showError(void)
{
    dpiErrorInfo info;

    dpiContext_getError(gContext, &info);
    fprintf(stderr, "ERROR: %.*s (%s: %s)\n", info.messageLength, info.message,
            info.fnName, info.action);
    return -1;
}

int main(int argc, char **argv)
{
    dpiData *inOutValue, *outValue, inValue;
    dpiVar *inOutVar, *outVar;
    uint32_t numQueryColumns;
    dpiStmt *stmt;
    dpiConn *conn;

    // connect to database and create statement
    conn = dpiSamples_getConn(0, NULL);
    if (dpiConn_prepareStmt(conn, 0, SQL_TEXT, strlen(SQL_TEXT), NULL, 0,
            &stmt) < 0)
        return dpiSamples_showError();

    // bind IN value
    inValue.isNull = 0;
    inValue.value.asBytes.ptr = "In value for testing";
    inValue.value.asBytes.length = strlen("In value for testing");
    if (dpiStmt_bindValueByPos(stmt, 1, DPI_NATIVE_TYPE_BYTES, &inValue) < 0)
        return dpiSamples_showError();

    // bind IN/OUT variable
    if (dpiConn_newVar(conn, DPI_ORACLE_TYPE_NUMBER, DPI_NATIVE_TYPE_INT64, 1,
            0, 0, 0, NULL, &inOutVar, &inOutValue) < 0)
        return dpiSamples_showError();
    inOutValue->isNull = 0;
    inOutValue->value.asInt64 = 347;
    if (dpiStmt_bindByPos(stmt, 2, inOutVar) < 0)
        return dpiSamples_showError();

    // bind OUT variable
    if (dpiConn_newVar(conn, DPI_ORACLE_TYPE_NUMBER, DPI_NATIVE_TYPE_INT64, 1,
            0, 0, 0, NULL, &outVar, &outValue) < 0)
        return dpiSamples_showError();
    if (dpiStmt_bindByPos(stmt, 3, outVar) < 0)
        return dpiSamples_showError();

    // perform execution
    if (dpiStmt_execute(stmt, 0, &numQueryColumns) < 0)
        return dpiSamples_showError();

    // display value of IN/OUT variable
    printf("IN/OUT value (after call) is %" PRId64 "\n",
            inOutValue->value.asInt64);
    dpiVar_release(inOutVar);

    // display value of OUT variable
    printf("OUT value (after call) is %" PRId64 "\n", outValue->value.asInt64);
    dpiVar_release(outVar);

    // clean up
    dpiStmt_release(stmt);
    dpiConn_release(conn);

    printf("Done.\n");
    return 0;
}
