let project = new Project('Matrix-Test');

await project.addProject(findKore());

project.addFile('sources/**');
project.setDebugDir('deployment');

project.flatten();

resolve(project);
