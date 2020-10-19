#!/bin/env python2.7

import argparse
import gitlab
import sys,os
import time

# CI_API_V4_URL=https://gitlab.cern.ch/api/v4
gitlabAPIURL = os.getenv('CI_API_V4_URL','CI_API_V4_URL')
gitlabURL = gitlabAPIURL.strip("/api/v4")

projectID = os.getenv('CI_PROJECT_ID','CI_PROJECT_ID')
jobToken  = os.getenv('CI_JOB_TOKEN','CI_JOB_TOKEN')
jobID     = os.getenv('CI_JOB_ID','CI_JOB_ID')

## Set these up globally, they'll only be used in MRs
# Maybe easier than doing the auth each function?
gl = None
mr = None
project = None

def rebase_mr(mr):
    ## do a rebase?? fancy!
    # mr.rebase()
    pass

def code_quality_message(failure, qtype):
    if qtype not in ["tidy","cppcheck","summary"]:
        pass
    pass

def code_format_message(failure):
    fileName = os.getenv('PATCHFILE','clang-format.patch')
    filePath = "{}/{}".format(os.getenv('ARTIFACTS_DIR','artifacts'),fileName)
    mrHash = os.getenv('CI_COMMIT_SHORT_SHA','CI_COMMIT_SHORT_SHA')

    mr_message = ""
    if not failure:
        mr_message = """### Code formatting report:
##### No issues detected

Congratulations! Your changes are correctly formatted. Thank you for this!
"""
    else:
        mr_message ="""### Code formatting report:
##### Issues detected

Your changes may need some additional formatting.


Please check `{0}` for the needed formatting changes.


#### Obtain the patch with the suggested changes
In your local working area at commit `{1}`, download the patch artifact:


##### Via the GUI
Click the "Download" link in the "Job Artifacts" section and copy the patch file to your working area.


##### Via the command line
To use this method, you will need to have created a [personal access token](https://docs.gitlab.com/ee/user/profile/personal_access_tokens.html), which you can do from [here](https://gitlab.cern.ch/profile/personal_access_tokens)
```sh
curl --location --header 'PRIVATE-TOKEN: <your_access_token>' "{2}/projects/{3}/jobs/{4}/artifacts/{5}"
```


#### Apply the patch with the suggested changes
**N.B. If you have already made local changes, please wait for this report to be generated again, or locally run `clang-format`.**


If your working area is identical to the commit submitted for the merge request you can execute:
```sh
git apply --check {0}
git am --signoff < {0}
```

""".format(fileName, mrHash, gitlabAPIURL, projectID, jobID, filePath, jobToken)

    mr_message = mr_message

    return mr_message

def get_mr_note(msg_id):
    """Find MR note from specified merge request with message identifier.
If more than one note is found, return error.
"""
    matched_ids = []

    for note in mr.notes.list():
        if note.body.find(msg_id) > -1:
            matched_ids.append(note)
            if note.body.find("##### Issues detected") > -1:
                status = "failing"
            else:
                status = "passing"
    if len(matched_ids) > 1:
        raise Exception("Found more than one matching note, unable to determine which to modify")
    if len(matched_ids) == 0:
        raise Exception("Found no matching notes")
    return matched_ids[0],status


def main(args):
    mr_message = ""
    if args.check == "format":
        mr_message = code_format_message(args.fail)
    elif args.check == "tidy":
        pass
    elif args.check == "cppcheck":
        pass
    elif args.check == "iwyu":
        pass
    elif args.check == "summary":
        if not args.fail:
            print("didn't fail")
        else:
            print("failed")
        pass

    print(mr_message)
    if os.getenv('CI_MERGE_REQUEST_ID'):
        global gl
        global mr
        global project

        try:
            gl = gitlab.Gitlab(gitlabURL,private_token=os.environ.get('PERSONAL_ACCESS_TOKEN'))
            print("gl",gl)
        except Exception as e:
            print("Unable to authenticate for API access: ", e)
            exit(1)

        ## get the project
        try:
            project = gl.projects.get(projectID)
            print("project", project)
        except Exception as e:
            print("Unable to find the project: ", e)
            exit(1)

        ## get the MR
        try:
            mr_id = os.getenv('CI_MERGE_REQUEST_IID')
            mr = project.mergerequests.get(mr_id)
            print("mr",mr)

            print(gl)
            print(mr)
            print(project)
            ## update if note already?
            try:
                # note_id = get_mr_note(mr_message.partition('\n')[0])
                # mr_note = mr.notes.get(note_id)
                mr_note,status = get_mr_note(mr_message.partition('\n')[0])


                if status == "failing" and mr_message.find("##### No issues detected") > -1:
                    status_change = "Issues resolved"
                elif status == "passing" and mr_message.find("##### Issues detected") > -1:
                    status_change = "Issues introduced since previous check"
                else:
                    status_change = "No change since last check"

                last_updated = """
##### Last updated: {}

{}
""".format(time.asctime(time.localtime()), status_change)
                mr_message = mr_message + last_updated
                mr_note.body = mr_message
                mr_note.save()
            except Exception as e:
                print("Creating new MR note: ",e)
                ## create new note
                mr_note = mr.notes.create({'body': mr_message})

        except Exception as e:
            print("Unable to post note to MR: ", e)
            exit(1)

    else:
        print("Not a merge request")
    pass

if __name__ == '__main__':

    KNOWN_CHECKS = {"format":"### Code formatting report:",
                    "tidy": "",
                    "cppcheck": "",
                    "iwyu": "",
                    "summary": ""}

    parser = argparse.ArgumentParser()
    parser.add_argument("check",help="Merge request message type")
    parser.add_argument("--fail",help="Check failed handler", action='store_true')
    parser.parse_args()
    args = parser.parse_args()

    if args.check not in KNOWN_CHECKS:
        print("Unknown message requested: {}".format(args.check))
    else:
        main(args)
