function(doc) {
    emit(null, {
        'userID': doc.userID,
        'studyID': doc.studyID,
        'trialID': doc.trialID,
        'skillLevel': doc.skillLevel,
        'id': doc._id
    });
}